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
	distance_z_( -(eye_distance+1.0F) )
{}


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

void  scene::get_matrix(glm::mat4 &prj, glm::mat4& mv) const
{
	prj = glm::frustum( -width_,width_, -height_,height_, eye_distance_, depth_);

	prj = glm::translate(prj, glm::vec3(0.0F, 0.0F, distance_z_));

	mv = glm::lookAt(
			 glm::vec3(0.0F,0.0F,1.0F),
			 glm::vec3(0.0F,0.0F,0.0F),
			 glm::vec3(0.0F,1.0F,0.0F));

	mv = glm::rotate(mv, angle_x_, glm::vec3(-1.0F, 0.0F, 0.0F));
	mv = glm::rotate(mv, angle_y_, glm::vec3(0.0F, 1.0F, 0.0F));

}

} // namespace engine
