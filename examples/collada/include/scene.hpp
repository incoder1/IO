#ifndef __SCENE_HPP_INCLUDED__
#define __SCENE_HPP_INCLUDED__

#ifdef _OPENMP
#	include <omp.h>
#endif

#include <glm/gtx/transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include "renderer.hpp"

namespace engine  {

class scene {
public:

	static scene perspective(float fov_y, float aspect, float z_near, float z_far) noexcept;

	scene(float width, float height,float eye_distance,float depth);

	void update_view_perspective(int widht,int height, float fov_y_rad) noexcept;

	void rotate_model(float x_rad, float y_rad) noexcept;

	void move_model(float distance) noexcept;

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
