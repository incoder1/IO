#ifndef __RENDERER_HPP_INCLUDED__
#define __RENDERER_HPP_INCLUDED__

namespace engine {

/// Material parameters to be used by GPU shader
/// 4 vectors matrix + shines
struct material_t {
	/// 0-3: ambient, 4-7: diffuse, 8-11: specular, 12-15: emission
	float adse[16];
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

// White plastic material
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
union light_t {
	/// 0-3: position, 4-7: ambient color, 8-11: diffuse color , 12-15: specular
	float pads[16];
};

static constexpr const light_t DEFAULT_LIGHT = {
	0.0F,0.0F,1.0F,0.0F,
	0.0F,0.0F,0.0F,1.0F,
	1.0F,1.0F,1.0F,1.0F,
	1.0F,1.0F,1.0F,0.0F
};

} // namespace engine

#endif // __RENDERER_HPP_INCLUDED__
