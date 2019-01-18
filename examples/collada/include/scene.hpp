#ifndef __SCENE_HPP_INCLUDED__
#define __SCENE_HPP_INCLUDED__

#ifdef _OPENMP
#	include <omp.h>
#endif

// force avx2 even for debug
#if defined(__GNUG__)
#	ifdef __AVX2__
#		define GLM_FORCE_AVX2
#	endif // __AVX2__
#	ifdef __AVX__
#		define GLM_FORCE_AVX
#	endif // __AVX__
#	ifdef __SSE4_2__
#		define GLM_FORCE_SSE42
#	endif
#	ifdef __SSE3__
#		define GLM_FORCE_SSE3
#	endif // __SSE3__
#	ifdef __SSE2__
#		define GLM_FORCE_SSE2
#	endif
#endif // defined

#include <glm/gtx/transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include "renderer.hpp"

namespace engine  {

class scene {
public:
	scene(float width, float height,float eye_distance,float depth);

	void rotate_model(float x_rad, float y_rad);

	void move_model(float distance);

	void move_light(float x, float y, float z) noexcept;

	void get_matrix(glm::mat4 &prj, glm::mat4& mv) const noexcept;

	const light_t light() const noexcept {
		return light_;
	}

private:

	float width_;
	float height_;
	float eye_distance_;
	float depth_;

	float angle_x_;
	float angle_y_;
	float distance_z_;

	light_t light_;
};

} // namespace engine

#endif // __SCENE_HPP_INCLUDED__
