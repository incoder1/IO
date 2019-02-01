#include "stdafx.hpp"
#include "scene.hpp"

namespace engine  {

// scene

scene scene::glu_perspective(float fov_y_rad, float aspect, float z_near, float z_far) noexcept
{
	const float height = std::tan(fov_y_rad) * z_near;
	const float width = height * aspect;
	return scene (width, height, z_near, z_far);
}

scene scene::perspective(int width, int height, float z_near, float z_far)
{
	const float fov_y_tan = static_cast<float>(height) / static_cast<float>(width);
	const float aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
	const float h = fov_y_tan * z_near;
	const float w = h * aspect_ratio;
	return scene(w, h, z_near, z_far);
}

scene::scene(float width, float height,float eye_distance,float depth):
	width_(width),
	height_(height),
	eye_distance_(eye_distance),
	depth_(depth),
	angle_x_(0.0f),
	angle_y_(0.0F),
	distance_z_( 0.0F ),
	light_(DEFAULT_LIGHT)
{
	light_.pads[2] = eye_distance;
}


void scene::rotate_model(float x_rad, float y_rad) noexcept
{
	// rotate axis
	angle_x_ = x_rad;
	angle_y_ = y_rad;
}


void scene::update_view_perspective(int width,int height) noexcept
{
	float aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
	float tan_fov_y = static_cast<float>(height) / static_cast<float>(width);
	height_ = tan_fov_y * eye_distance_;
	width_ = height_ * aspect_ratio;
}

void scene::move_model(float distance) noexcept
{
	distance_z_ = distance;
}

void scene::move_light(float x_delta, float y_delta, float z_delta) noexcept
{
	light_.pads[0] += x_delta;
	light_.pads[1] += y_delta;
	light_.pads[2] += z_delta;
}

void  scene::get_matrix(glm::mat4 &prj, glm::mat4& mv) const noexcept
{
	prj = glm::frustum( -width_,width_, -height_,height_, eye_distance_, depth_);

	if(0.0F != distance_z_)
		prj = glm::translate(prj, glm::vec3(0.0F, 0.0F, distance_z_) );

	// load identity
	mv = glm::mat4(1.0f);

	static glm::vec3 up_x(-1.0F, 0.0F, 0.0F);
	static glm::vec3 up_y(0.0F, 1.0F, 0.0F);

	mv = glm::rotate(mv, angle_x_, up_x);
	mv = glm::rotate(mv, angle_y_, up_y);
}


} // namespace engine
