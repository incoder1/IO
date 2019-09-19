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
