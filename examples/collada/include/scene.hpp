#ifndef __SCENE_HPP_INCLUDED__
#define __SCENE_HPP_INCLUDED__

#ifdef _OPENMP
#	include <omp.h>
#endif

#ifndef GLM_ENABLE_EXPERIMENTAL
#   define GLM_ENABLE_EXPERIMENTAL 1
#endif // GLM_ENABLE_EXPERIMENTAL

#include <glm/gtx/transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include "renderer.hpp"

namespace engine  {

/// Rendering scene
class scene {
public:

	/// Creates with gluPerspective similar functionality
	static scene glu_perspective(float fov_y, float aspect, float z_near, float z_far) noexcept;

	/// Creates with gluPerspective similar functionality, with auto-calculate aspect and fov_y
	/// for the view-port
	static scene perspective(int width, int height, float z_near, float z_far);

	/// Creates scene with frustum perspective
	/// \param width scene width from the eye position (center of the screen) to the left and right limit plane
	/// \param height scene height from the eye position (center of the screen) to the top and bottom limit plane
	/// \param eye_distance distance from the center of the scene ({0,0,0}) to the user eye
	/// \param depth distance from the center of the scene to the backward limit plane
	scene(float width, float height,float eye_distance,float depth);

	/// Updates the perspective when view-port changed
	/// \param width new view port width in pixels
	/// \param height new view port height in pixels
	void update_view_perspective(int width,int height) noexcept;

	/// Rotates model on x and y axis
	/// \param x_rad x axis rotation angle in radians
	/// \param y_rad y axis rotation angle in radians
	void rotate_model(float x_rad, float y_rad) noexcept;

	/// Translate model on z axis
	/// \param distance an offset by z axis
	void move_model(float distance) noexcept;

	/// Sets light direction or position
	/// \param x light x axis value
	/// \param y light y axis value
	/// \param z light z axis value
	/// \param w light direction vector when 0.0F or position if 1.0F
	void set_light(float x, float y, float z, float w) noexcept;

	/// Move light direction or position
	/// \param x_delta light x axis direction or position delta
	/// \param y_delta light y axis direction or position delta
	/// \param z_delta light z axis direction or position delta
	void move_light(float x_delta, float y_delta, float z_delta) noexcept;

	/// Get OpenGL LH frustum matrix
	/// \param prj projection matrix
    /// \param mv model view matrix
	void world(glm::mat4 &prj, glm::mat4& mv) const noexcept;

	/// Gets current light parameters
	/// \return light parameters
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
