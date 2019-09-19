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
#include "collada.hpp"

namespace collada {

// accessor
accessor::accessor(io::const_string&& src_id,std::size_t count, std::size_t stride):
	io::object(),
	source_id_( std::forward<io::const_string>(src_id)),
	count_(count),
	stride_(stride),
	layout_()
{
	layout_.reserve(3);
}

accessor::~accessor() noexcept
{}

void accessor::add_parameter(parameter&& prm)
{
	layout_.emplace_back( std::forward<parameter>(prm) );
}

// source
source::source():
	io::object(),
	float_arrays_(),
	accessors_()
{}

source::~source() noexcept
{}

void source::add_float_array(io::const_string&& id, float_array&& arr)
{
	auto ret = float_arrays_.emplace(
		std::forward<io::const_string>(id),
		std::forward<float_array>(arr) );
	if(!ret.second) {
		std::string msg("more then one float array with the same identifier: ");
		msg.append(id.data());
		throw std::runtime_error( msg );
	}
}

const float_array source::find_float_array(const io::const_string& id) const
{
	auto it = float_arrays_.find( id );
	return float_arrays_.cend() == it ? float_array() : it->second;
}


void source::add_accessor(s_accessor&& acsr)
{
	accessors_.emplace_back( std::forward<s_accessor>(acsr) );
}

// geometry
geometry::geometry(surface_type type,io::const_string&& name) noexcept:
	io::object( ),
	type_( type ),
	name_( std::forward<io::const_string>(name) )
{}

geometry::~geometry() noexcept
{}

// sub_mesh
sub_mesh::sub_mesh(primitive_type type, io::const_string&& mat, std::size_t count,input_library_t&& layout, unsigned_int_array&& primitive, byte_array&& vcount) noexcept:
	io::object(),
	type_(type),
	mat_(std::forward<io::const_string>(mat)),
	layout_(std::forward<input_library_t>(layout)),
	primitive_(std::forward<unsigned_int_array>(primitive)),
	vcount_(std::forward<byte_array>(vcount)),
	count_(count)
{}

sub_mesh::~sub_mesh() noexcept
{}

// mesh
mesh::mesh(io::const_string&& name) noexcept:
	geometry(geometry::surface_type::mesh, std::forward<io::const_string>(name) ),
	pos_src_id_(),
	source_library_(),
	sub_meshes_()
{
}

mesh::~mesh() noexcept
{
}

void mesh::add_source(io::const_string&& id,s_source&& src)
{
	source_library_.emplace( std::forward<io::const_string>(id),
							 std::forward<s_source>(src) );
}

s_source mesh::find_souce(const io::const_string& id) const
{
	auto it = source_library_.find( id );
	return source_library_.cend() == it ? s_source() : it->second;
}

// scene
scene::scene(io::const_string&& id,io::const_string&& name):
	io::object(),
	id_( std::forward<io::const_string>(id) ),
	name_( std::forward<io::const_string>(name) ),
	nodes_()
{}

scene::~scene() noexcept
{}

void scene::add_node(node&& nd)
{
	nodes_.emplace_back( std::forward<node>(nd) );
}

// effect_library
effect_library::effect_library():
	io::object(),
	effects_(),
	surfaces_(),
	sampler_refs_()
{}

void effect_library::add_effect(io::const_string&& id,effect&& e)
{
	effects_.emplace( std::forward<io::const_string>(id), std::make_shared<effect>( std::forward<effect>(e) ) );
}

std::shared_ptr<effect> effect_library::find_effect(const io::const_string& id) const noexcept
{
	effects_lib_t::const_iterator it = effects_.find( id );
	return effects_.cend() == it ? std::shared_ptr<effect>() : it->second;
}

void effect_library::add_surface(io::const_string&& id,surface&& sf)
{
	surfaces_.emplace( std::forward<io::const_string>(id), std::forward<surface>(sf) );
}

std::pair<bool,surface> effect_library::find_surface(const io::const_string& id) const noexcept
{
	surfaces_lib_t::const_iterator it = surfaces_.find( id );
	return surfaces_.cend() == it ? std::make_pair(false,surface()) : std::make_pair( true, it->second );
}

void effect_library::add_sampler_ref(io::const_string&& id,io::const_string&& sid)
{
	sampler_refs_.emplace(
				std::forward<io::const_string>(id),
				std::forward<io::const_string>(sid)
			);
}

io::const_string effect_library::find_sampler_ref(const io::const_string& id) const noexcept
{
	sampler_refs_lib_t::const_iterator it = sampler_refs_.find( id );
	return sampler_refs_.cend() != it ? it->second : io::const_string();
}

// model
model::model():
	io::object(),
	effects_( new effect_library() ),
	images_(),
	meshes_(),
	materials_(),
	scene_()
{}

model::~model() noexcept
{}


void model::add_geometry(io::const_string&& id,s_geometry&& e)
{
	meshes_.emplace( std::forward<io::const_string>(id), std::forward<s_geometry>(e) );
}

s_geometry model::find_geometry(const io::const_string& id) noexcept
{
	geometry_library_t::const_iterator it = meshes_.find( id );
	return meshes_.cend() == it ? s_mesh() : it->second;
}

void  model::add_material_effect_link(io::const_string&& id,io::const_string&& eff)
{
	materials_.emplace( std::forward<io::const_string>(id), std::forward<io::const_string>(eff) );
}

std::shared_ptr<effect> model::find_material(const io::const_string& id) const noexcept
{
	material_library_t::const_iterator it = materials_.find( id );
	if( materials_.cend() != it ) {
		return effects_->find_effect( it->second );
	}
	return std::shared_ptr<effect>();
}

void model::add_image(io::const_string&& id,io::const_string&& img)
{
	images_.emplace( std::forward<io::const_string>(id), std::forward<io::const_string>(img) );
}

io::const_string model::find_image(const io::const_string& id) noexcept
{
	image_library_t::const_iterator it = images_.find( id );
	return ( images_.end() != it ) ? it->second : io::const_string();
}


} // namespace collada
