#include "stdafx.hpp"
#include "model.hpp"

namespace engine {

// model
model::model():
	io::object(),
	surfaces_()
{}

model::~model() noexcept
{}

void model::add_surface(s_surface&& srf)
{
	surfaces_.emplace_front( std::forward<s_surface>(srf) );
}

void model::render(const scene& scn) const
{
	std::for_each(surfaces_.cbegin(), surfaces_.cend(), [scn](const s_surface& srf) {
		srf->draw( scn );
	} );
}

} // namespace engine
