#include "stdafx.hpp"
#include "collada.hpp"

namespace collada {

// source
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

void source::add_accessor(accessor&& acsr)
{
	accessors_.emplace_back( std::make_shared<accessor>( std::forward<accessor>(acsr) ) );
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

void model::add_mesh(io::const_string&& id,mesh&& e)
{
	meshes_.emplace( std::forward<io::const_string>(id),  std::make_shared<mesh>( std::forward<mesh>(e) ) );
}

std::shared_ptr<mesh> model::find_mesh(const char* id) noexcept
{
	geometry_library_t::const_iterator it = meshes_.find( io::const_string(id) );
	return meshes_.cend() == it ? std::shared_ptr<mesh>() : it->second;
}

} // namespace collada
