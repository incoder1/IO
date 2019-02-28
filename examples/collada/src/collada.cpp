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
	float_arrays_.emplace(
		std::forward<io::const_string>(id),
		std::forward<float_array>(arr)
	);
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

// index_data
index_data::index_data() noexcept:
        io::object(),
        primitives_(primitive_type::triangles),
        count_(0),
        indices_()
{}

index_data::~index_data() noexcept
{}

// mesh
mesh::mesh(io::const_string&& name) noexcept:
	io::object(),
	name_( std::forward<io::const_string>(name) ),
	vertex_id_(),
	source_library_(),
	input_channels_(),
	index_( new index_data() )
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

void mesh::add_input_channel(input_channel&& ich)
{
 	input_channels_.emplace_back( std::forward<input_channel>(ich) );
}

// model
model::model():
	effects_(),
	images_(),
	meshes_()
{}

void model::add_effect(io::const_string&& id,effect&& e)
{
	effects_.emplace( std::forward<io::const_string>(id), std::make_shared<effect>( std::forward<effect>(e) ) );
}

std::shared_ptr<effect> model::find_effect(const char* id) noexcept
{
	effect_library_t::const_iterator it = effects_.find( io::const_string(id) );
	return effects_.cend() == it ? std::shared_ptr<effect>() : it->second;
}

void model::add_mesh(io::const_string&& id,s_mesh&& e)
{
	meshes_.emplace( std::forward<io::const_string>(id), std::forward<s_mesh>(e) );
}

s_mesh model::find_mesh(const char* id) noexcept
{
	geometry_library_t::const_iterator it = meshes_.find( io::const_string(id) );
	return meshes_.cend() == it ? s_mesh() : it->second;
}

} // namespace collada
