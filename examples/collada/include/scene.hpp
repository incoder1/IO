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

	/// Creates with gluPerspective similar functionality
	static scene glu_perspective(float fov_y, float aspect, float z_near, float z_far) noexcept;

	/// Creates with gluPerspective similar functionality, with auto-calculate aspect and fov_y
	/// for the view-port
	static scene perspective(int width, int height, float z_near, float z_far);

	/// Creates scene with frustum perspective
	scene(float width, float height,float eye_distance,float depth);

	/// Update the perspective when view-port changed
	void update_view_perspective(int widht,int height) noexcept;

	/// Rotate model on x and y axis
	void rotate_model(float x_rad, float y_rad) noexcept;

	/// Translate model on z axis
	void move_model(float distance) noexcept;

	/// Move light position
	void move_light(float x, float y, float z) noexcept;

	/// Get OpenGL LH frustum matrix
	void get_frustum(glm::mat4 &prj, glm::mat4& mv) const noexcept;

	/// Light parameters
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
