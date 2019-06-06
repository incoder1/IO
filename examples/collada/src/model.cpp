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
	for(auto s: surfaces_) {
		s->draw( scn );
	}
}

//

} // namespace engine
