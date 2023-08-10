/*
 *
 * Copyright (c) 2016-2019
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#include "stdafx.hpp"
#include "model_loader.hpp"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <cmath>

namespace engine {

namespace detail {

// obtains parsed float array from COLLADA source
static void map_accessor(const collada::s_source& src,detail::vertex_accessor_builder& dst)
{
	collada::s_accessor src_acsr = * ( src->cbegin() );
	dst.set_stride( src_acsr->stride() );
	dst.set_array( src->find_float_array( src_acsr->source_id() ) );
}

// static_mesh_accessor
static_mesh_accessor::static_mesh_accessor(vertex_accessor&& pos, vertex_accessor&& nrm, collada::unsigned_int_array&& idx):
   position_( pos ),
   normale_( nrm ),
   idx_( idx ),
   vertex_count_( 0 ),
   vertex_size_( 0 )
{
   vertex_count_ = idx_.length() / 2;
   vertex_size_ = position_.stride() + normale_.stride();
}

void static_mesh_accessor::copy_vertex(float* dst, std::size_t vertex_index)
{
	const std::size_t src_idx = vertex_index * 2;
	const std::size_t pos_idx = idx_[ src_idx + position_.offset() ];
	const std::size_t nrm_idx = idx_[ src_idx + normale_.offset() ];
	position_.copy(dst, pos_idx );
	normale_.copy( (dst + position_.stride() ), nrm_idx );
}

//textured_mesh_accessor
textured_mesh_accessor::textured_mesh_accessor(vertex_accessor&& pos, vertex_accessor&& nrm, vertex_accessor&& uv, collada::unsigned_int_array&& idx):
   position_( pos ),
   normale_( nrm ),
   uv_( uv ),
   idx_( idx ),
   vertex_count_( 0 ),
   vertex_size_( 0 )
{
   vertex_count_ = idx.length() / 3;
   vertex_size_ = position_.stride() + normale_.stride() + uv_.stride();
}

void textured_mesh_accessor::copy_vertex(float* dst, std::size_t vertex_index)
{
	const std::size_t src_idx = vertex_index * 3;
	const std::size_t pos_idx = idx_[ src_idx + position_.offset() ];
	const std::size_t nrm_idx = idx_[ src_idx + normale_.offset() ];
	const std::size_t uv_idx = idx_[ src_idx + uv_.offset() ];
	position_.copy(dst, pos_idx );
	dst += position_.stride();
	normale_.copy( dst, nrm_idx );
	dst += normale_.stride();
	uv_.copy( dst, uv_idx );
}

static void tangent_vector(float * face,const std::size_t src_vetex_size)
{
	// 3 vertex per triangle
	// vertex layout is
	// {pox_x,pos_y,pos_z},{nrm_x, nrm_y, nrm_z},{uv_s,uv_t},{tan_x,tan_y,tan_z}
	// bi-tangent will be calculated by vertex shader

	const std::size_t full_vertex_size = src_vetex_size + 3;

	const std::size_t v1_base = 0;
	const std::size_t v2_base = full_vertex_size;
	const std::size_t v3_base = full_vertex_size*2;

	constexpr std::size_t uv_offset = 6;

	const std::size_t v1_uv_base = v1_base+uv_offset;
	const std::size_t v2_uv_base = v2_base+uv_offset;
	const std::size_t v3_uv_base = v3_base+uv_offset;


	// triangle vertex positions
	glm::vec3 pos1(face[v1_base], face[v1_base+1], face[v1_base+2]);
	glm::vec3 pos2(face[v2_base], face[v2_base+1], face[v2_base+2]);
	glm::vec3 pos3(face[v3_base], face[v3_base+1], face[v3_base+2]);

	// triangle vertex texture mapping UV's
	glm::vec2 uv1(face[v1_uv_base], face[v1_uv_base+1]);
	glm::vec2 uv2(face[v2_uv_base], face[v2_uv_base+1]);
	glm::vec2 uv3(face[v3_uv_base], face[v3_uv_base+1]);

	glm::vec3 edge1 = pos2 - pos1;
	glm::vec3 edge2 = pos3 - pos1;

	glm::vec2 delta_uv1 = uv2 - uv1;
	glm::vec2 delta_uv2 = uv3 - uv1;

	// tangent
	const float f = 1.0F / ( (delta_uv1.x * delta_uv2.y) - (delta_uv2.x * delta_uv1.y) );
	float x = f * ( (delta_uv2.y * edge1.x) - (delta_uv1.y * edge2.x) );
	float y = f * ( (delta_uv2.y * edge1.y) - (delta_uv1.y * edge2.y) );
	float z = f * ( (delta_uv2.y * edge1.z) - (delta_uv1.y * edge2.z) );

	// normalize
	float inv_length = 1.0F / std::sqrt( (x * x) + (y * y) + (z * z) );
	float tangent[3] = {x * inv_length, y * inv_length, z * inv_length};

	const std::size_t v1_tan_base = src_vetex_size;
	const std::size_t v2_tan_base = v2_base + src_vetex_size;
	const std::size_t v3_tan_base = v3_base + src_vetex_size;

	// fill tangents for all triangle vertexes
	std::memcpy( face+v1_tan_base, tangent, sizeof(tangent) );
	std::memcpy( face+v2_tan_base, tangent, sizeof(tangent) );
	std::memcpy( face+v3_tan_base, tangent, sizeof(tangent) );

}


} // namespace detail

//model_loader

collada::s_model model_loader::parse_collada_xml(const io::file& dae_file)
{
	std::error_code ec;
	io::s_read_channel src = dae_file.open_for_read(ec);
	io::check_error_code(ec);
	collada::parser psr( std::move(src) );
	return psr.load();
}

model_loader::model_loader(const io::file& src):
	src_(src),
	src_mdl_( parse_collada_xml(src) ),
	textures_()
{
}

model_loader::~model_loader() noexcept
{
}

detail::static_mesh_accessor model_loader::create_static_mesh_accessor(const collada::s_sub_mesh& sm, const collada::mesh* src_mesh)
{
	using namespace collada;
	detail::vertex_accessor_builder pos, nrm;
	for(input it: sm->layout() ) {
		io::const_string acr_id = it.accessor_id;
		switch(it.type) {
		case semantic_type::vertex:
			pos.set_offset(it.offset);
			detail::map_accessor( src_mesh->find_souce( src_mesh->pos_src_id() ), pos );
			break;
		case semantic_type::normal:
			nrm.set_offset(it.offset);
			detail::map_accessor( src_mesh->find_souce(acr_id), nrm);
			break;
		default:
			break;
		}
	}

	return detail::static_mesh_accessor(
							pos.build(),
							nrm.build(),
							sm->primitive()	);
}

detail::textured_mesh_accessor model_loader::create_textured_mesh_accessor(const collada::s_sub_mesh& sm, const collada::mesh* src_mesh)
{
	using namespace collada;
	detail::vertex_accessor_builder pos, nrm, uv;
	// loop over the input and look-up sources
	for(input it: sm->layout() ) {
		io::const_string acr_id = it.accessor_id;
		switch(it.type) {
		case semantic_type::vertex:
			pos.set_offset(it.offset);
			detail::map_accessor( src_mesh->find_souce( src_mesh->pos_src_id() ), pos );
			break;
		case semantic_type::normal:
			nrm.set_offset(it.offset);
			detail::map_accessor( src_mesh->find_souce(acr_id), nrm);
			break;
		case semantic_type::texcoord:
			uv.set_offset(it.offset);
			map_accessor( src_mesh->find_souce(acr_id), uv);
			break;
		default:
			break;
		}
	}

	return detail::textured_mesh_accessor(pos.build(), nrm.build(), uv.build(), sm->primitive()	);
}

s_surface model_loader::load_static_sub_mesh(const collada::s_sub_mesh& sm, const collada::mesh* src_mesh, material_t&& mat)
{

	detail::static_mesh_accessor acr = create_static_mesh_accessor(sm, src_mesh);

	const std::size_t vertex_count = acr.vertex_count();
	const std::size_t vertex_size = acr.vertex_size();
	io::scoped_arr<float> vbo( vertex_count * vertex_size );

	#ifdef _OPENMP
	#pragma omp parallel for simd
	#endif // _OPENMP
	for(std::size_t i=0; i < vertex_count; i++) {
        acr.copy_vertex( &vbo[ i * vertex_size], i);
	}

	return s_surface(
			   new geometry_mesh(
							sufrace_data(
									mat,vertex_count,
				   					util::array_view<float>(vbo.begin(),vbo.len()),
									util::array_view<unsigned int>()
									)
								)
				);
}

gl::s_texture model_loader::get_texture_by_name(const io::const_string& name)
{
	auto it = textures_.find(name);
	if( textures_.end() != it)
		return it->second;
	// load texture and put int into pool
	const std::string path = src_.path();
	std::string base_dir = path.substr(0, path.rfind( io::file::separator() ) + 1 );
	io::file image_file( base_dir.append( name.data() ) );

	if( !image_file.exist() )
		throw std::runtime_error( name.clone().append(" file is not found") );

	// TODO: Load different types of images according file extension
	s_image timg = engine::load_png_rgba(image_file, true);
	gl::s_texture ret = gl::texture::create_texture2d_from_image(
							timg,
							gl::texture_filter::LINEAR_MIPMAP_LINEAR);
	textures_.emplace( name, ret );
	return ret;
}

s_surface model_loader::load_textured_sub_mesh(const collada::s_sub_mesh& sm, const collada::mesh* mesh,const io::const_string& texture)
{

	detail::textured_mesh_accessor acr = create_textured_mesh_accessor(sm, mesh);

	gl::s_texture tex = get_texture_by_name(texture);

	const std::size_t vertex_count = acr.vertex_count();
	const std::size_t vertex_size = acr.vertex_size();
	io::scoped_arr<float> vbo( vertex_count * vertex_size );


	#ifdef _OPENMP
	#pragma omp parallel for simd
	#endif // _OPENMP
	for(std::size_t i=0; i < vertex_count; i++) {
        acr.copy_vertex( &vbo[ i * vertex_size], i);
	}

	return s_surface(
			   new textured_mesh(
					sufrace_data(
						DEFAULT_MATERIAL, vertex_count,
						util::array_view<float>(vbo.begin(), vbo.len()),
						util::array_view<unsigned int>()
					 ),
					tex
			   )
		   );
}


s_surface model_loader::load_bummaped_mesh(const collada::s_sub_mesh& sm, const collada::mesh* mesh,const io::const_string& diffuse,const io::const_string& bump)
{
	detail::textured_mesh_accessor acr = create_textured_mesh_accessor(sm, mesh);

	gl::s_texture diffuse_tex = get_texture_by_name(diffuse);
	gl::s_texture bumpmap = get_texture_by_name(bump);

	constexpr std::size_t TAN_STRIDE = 3;
	const std::size_t vertex_count = acr.vertex_count();
	// add tangent
	const std::size_t src_vertex_size = acr.vertex_size();
	const std::size_t dst_vertex_size = src_vertex_size + TAN_STRIDE;
	io::scoped_arr<float> vbo( vertex_count * dst_vertex_size );

	#ifdef _OPENMP
	#pragma omp parallel for simd
	#endif // _OPENMP
	for(std::size_t i=0; i < vertex_count; i += TAN_STRIDE) {
		// triangle face to calculate tangent vector
		float *face = vbo.begin() +  (i * dst_vertex_size);
		// copy face into tmp buff, and calculate tangent vector
        acr.copy_vertex( face , i);
		acr.copy_vertex( face + dst_vertex_size, i+1);
		acr.copy_vertex( face + (dst_vertex_size*2), i+2);
		// calculate tangent vector for triangle
		// and replicate it for each triangle vertex
		detail::tangent_vector( face, src_vertex_size );
	}

	return s_surface(
			new normal_mapped_mesh(
				sufrace_data(
				   PERL_MATERIAL, vertex_count,
				   util::array_view<float>(vbo.begin(), vbo.len()),
				   util::array_view<unsigned int>()
				),
				diffuse_tex,
				bumpmap
			   )
		   );
}

void model_loader::load_mesh(s_model& dst_mdl,const collada::mesh* src_mesh)
{
	using namespace collada;
	s_effect_library efl = src_mdl_->effects();
	for(auto it = src_mesh->cbegin(); it != src_mesh->cend(); ++it) {
		s_sub_mesh sm = *it;
		std::shared_ptr<effect> ef = src_mdl_->find_material( sm->material() );
		if(!ef)
			break;
		switch( ef->shade ) {
		case shade_type::blinn_phong:
		case shade_type::phong: {
			engine::material_t mat;
			std::memmove( mat.adse, ef->value.pong.adse.mat, sizeof(mat) );
			mat.shininess = ef->value.pong.shininess;
			dst_mdl->add_surface( load_static_sub_mesh( sm, src_mesh, std::move(mat) ) );
		}
		break;
		case shade_type::diffuse_texture: {
			// TODO: get texture type from surface type
			io::const_string sid = efl->find_sampler_ref( ef->diffuse_tex->name() );
			collada::surface srf = efl->find_surface( sid ).second;
			io::const_string diffuse_texture_file = src_mdl_->find_image( srf.init_from );
			dst_mdl->add_surface( load_textured_sub_mesh( sm, src_mesh, diffuse_texture_file));
		}
		break;
		case shade_type::bump_mapping: {
			io::const_string sid = efl->find_sampler_ref( ef->diffuse_tex->name() );
			collada::surface srf = efl->find_surface( sid ).second;
			io::const_string diffuse_texture_file = src_mdl_->find_image( srf.init_from );

			sid = efl->find_sampler_ref( ef->bumpmap_tex->name() );
			srf = efl->find_surface( sid ).second;
			io::const_string bump_texture_file = src_mdl_->find_image( srf.init_from );

			dst_mdl->add_surface( load_bummaped_mesh(sm, src_mesh, diffuse_texture_file, bump_texture_file) );
		}
		break;
		case shade_type::constant:
		case shade_type::lambert:
			break;
		}
	}
}

void model_loader::load(s_model& dst_mdl)
{
	// load parsed COLLADA model into renderer
	collada::s_scene src_scene = src_mdl_->scene();
	for(collada::scene::const_iterator it = src_scene->cbegin();
			it != src_scene->cend(); ++it ) {
		if( ! it->geo_ref.url.blank() ) {
			collada::s_geometry geo = src_mdl_->find_geometry( it->geo_ref.url );
#ifndef NDEBUG
			std::clog << "Geometry node name: " << geo->name().data() << std::endl;
#endif // NDEBUG
			load_mesh(dst_mdl, reinterpret_cast<collada::mesh*>( geo.get() ) );
		}
	}
}

} // namespace engine
