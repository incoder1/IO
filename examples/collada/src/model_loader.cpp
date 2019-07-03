#include "stdafx.hpp"
#include "model_loader.hpp"

namespace engine {

struct face_accessor {
	collada::float_array arr;
	std::size_t offset;
	std::size_t stride;
	constexpr face_accessor() noexcept:
		arr(),
		offset(0),
		stride(0)
	{}
	const float* get_vec( std::size_t index) noexcept {
		return &arr[ index * stride ];
	}
};

// obtains parsed float array from COLLADA source
static void map_accessor(const collada::s_source& src,face_accessor& dst)
{
	collada::s_accessor src_acsr = * ( src->cbegin() );
	dst.stride = src_acsr->stride();
	dst.arr = src->find_float_array( src_acsr->source_id() );
}

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

s_surface model_loader::load_static_sub_mesh(const collada::s_sub_mesh& sm, const collada::mesh* src_mesh, material_t&& mat)
{

	using namespace collada;
	face_accessor pos_acr, nrm_acr;
	for(input it: sm->layout() ) {
		io::const_string acr_id = it.accessor_id;
		switch(it.type) {
		case semantic_type::vertex:
			pos_acr.offset = it.offset;
			map_accessor( src_mesh->find_souce( src_mesh->pos_src_id() ), pos_acr );
			break;
		case semantic_type::normal:
			nrm_acr.offset = it.offset;
			map_accessor( src_mesh->find_souce(acr_id), nrm_acr);
			break;
		default:
			break;
		}
	}

	unsigned_int_array idx = sm->index();
	const std::size_t vertex_count = idx.length() / 2;

	const std::size_t face_size = pos_acr.stride + nrm_acr.stride;
	io::scoped_arr<float> vbo( vertex_count * face_size );

	for(std::size_t i=0, offset = 0; i < idx.length(); i += 2, offset += face_size) {
		const float *pos_v =  pos_acr.get_vec( idx[ i + pos_acr.offset ] );
		const float *nrm_v =  nrm_acr.get_vec( idx[ i + nrm_acr.offset ] );
		float face[face_size];
		std::memmove( face, pos_v,  pos_acr.stride * sizeof(float) );
		std::memmove( &face[3], nrm_v,  nrm_acr.stride * sizeof(float) );
		std::memmove( &vbo[offset], face, sizeof(face) );
	}

	return s_surface(
			   new geometry_mesh(
				   mat,
				   vbo.get(), vbo.len(),
				   nullptr, vertex_count
			   )
		   );

}

gl::s_texture model_loader::get_texture_by_name(const io::const_string& name)
{
	auto it = textures_.find(name);
	if( textures_.end() != it)
		return it->second;
	// load texture and put int into pool
	// TODO: Add path from base directory
	io::file image_file( name.stdstr() );
	if( !image_file.exist() )
		throw std::runtime_error( name.stdstr().append(" file is not found") );

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
	using namespace collada;

	gl::s_texture tex = get_texture_by_name(texture);

	face_accessor pos_acr, nrm_acr, uv_acr;
	// loop over the input and look-up sources
	for(input it: sm->layout() ) {
		io::const_string acr_id = it.accessor_id;
		switch(it.type) {
		case semantic_type::vertex:
			pos_acr.offset = it.offset;
			map_accessor( mesh->find_souce( mesh->pos_src_id() ), pos_acr );
			break;
		case semantic_type::normal:
			nrm_acr.offset = it.offset;
			map_accessor( mesh->find_souce(acr_id), nrm_acr);
			break;
		case semantic_type::texcoord:
			uv_acr.offset = it.offset;
			map_accessor( mesh->find_souce(acr_id), uv_acr);
			break;
		default:
			break;
		}
	}


	unsigned_int_array idx = sm->index();
	const std::size_t vertex_count = idx.length() / 2;

	const std::size_t face_size = pos_acr.stride + nrm_acr.stride + uv_acr.stride;
	io::scoped_arr<float> vbo( vertex_count * face_size );

	for(std::size_t i=0, offset = 0; i < idx.length(); i += 3, offset += face_size) {
		const float *pos_v =  pos_acr.get_vec( idx [ i + pos_acr.offset ] );
		const float *nrm_v =  nrm_acr.get_vec( idx[ i + nrm_acr.offset ] );
		const float *uv_v =   uv_acr.get_vec( idx[ i + uv_acr.offset] );
		float face[face_size];
		std::memmove( face, pos_v,  pos_acr.stride * sizeof(float) );
		std::memmove( &face[3], nrm_v,  nrm_acr.stride * sizeof(float) );
		std::memmove( &face[6], uv_v,  uv_acr.stride * sizeof(float) );
		std::memmove( &vbo[offset], face, sizeof(face) );
	}

	return s_surface(
			   new textured_mesh(
				   DEFAULT_MATERIAL,
				   vbo.get(), vbo.len(),
				   nullptr, vertex_count,
				   tex
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
			io::const_string sid = efl->find_sampler_ref( ef->tex.name );
			collada::surface srf = efl->find_surface( sid ).second;
			io::const_string diffuse_texture_file = src_mdl_->find_image( srf.init_from );
			dst_mdl->add_surface( load_textured_sub_mesh( sm, src_mesh, diffuse_texture_file) );
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
