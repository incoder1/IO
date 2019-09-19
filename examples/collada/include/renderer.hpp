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
#ifndef __RENDERER_HPP_INCLUDED__
#define __RENDERER_HPP_INCLUDED__

namespace engine {

/// Material parameters to be used by GPU shader
/// 4 vectors matrix + shines
struct material_t {
	/// color components matrix:
	/// 0-3: ambient vector
	/// 4-7: diffuse vector
	/// 8-11: specular vector
	/// 12-15: emission vector
	float adse[16];
	/// Material shininess, should be multiplied on 128 for OpenGL
	float shininess;
};

/// Default OpenGL material
static constexpr const material_t DEFAULT_MATERIAL = {
	{
		0.2F, 0.2F, 0.2F, 1.0F,
		0.8F, 0.8F, 0.8F, 1.0F,
		0.0F, 0.0F, 0.0F, 1.0F,
		0.0F, 0.0F, 0.0F, 1.0F
	},
	0.0
};

/// White plastic material
static constexpr const material_t WHITE_PLASTIC_MATERIAL = {
	{
		0.0F,0.0F,0.0F,1.0F,
		0.55F,0.55F,0.55F,1.0F,
		0.7F,0.7F,0.7F,1.0F,
		0.0F,0.0F,0.0F,1.0F
	},
	32.0F
};

/// Light parameters  to be used by GPU shader
/// 4 vectors matrix
struct light_t {
	/// 0-3: position, 4-7: ambient color vector
	/// 8-11: diffuse color vector
	/// 12-15: specular color vector
	float pads[16];
};

/// Default OpenGL light position
static constexpr const light_t DEFAULT_LIGHT = {
	{
	 0.0F,0.0F,1.0F,0.0F,
	 0.0F,0.0F,0.0F,1.0F,
	 1.0F,1.0F,1.0F,1.0F,
	 1.0F,1.0F,1.0F,0.0F
	}
};

} // namespace engine

#endif // __RENDERER_HPP_INCLUDED__
