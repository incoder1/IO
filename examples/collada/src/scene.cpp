#include "stdafx.hpp"
#include "scene.hpp"

namespace engine  {

// scene
scene::scene(float width, float height,float eye_distance,float depth):
	width_(width),
	height_(height),
	eye_distance_(eye_distance),
	depth_(depth),
	angle_x_(0.0f),
	angle_y_(0.0F),
	distance_z_( -(eye_distance+1.0F) ),
	light_(DEFAULT_LIGHT)
{
	light_.pads[2] = eye_distance;
}


void scene::rotate_model(float x_rad, float y_rad)
{
	// rotate axis
	angle_x_ = x_rad;
	angle_y_ = y_rad;
}

void scene::move_model(float distance)
{
	distance_z_ = distance;
}

void scene::move_light(float x, float y, float z) noexcept
{
	light_.pads[0] = x;
	light_.pads[1] = y;
	light_.pads[2] = z;
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
