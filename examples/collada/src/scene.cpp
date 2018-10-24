#include "scene.hpp"

namespace engine  {

// scene
scene::scene(float width, float height,float eye_distance,float depth):
	model_(1.0f),
	view_( glm::frustum(
			-width, width,
			-height, height,
			eye_distance,
			depth) ),
	light_position_(0.0f, 0.0f, eye_distance+1.0f),
	modelx_(0.0f),
	modely_(0.0f),
	modelz_(0.0f)
{
}


void scene::rotate_model(float x_rad, float y_rad, float z_rad)
{
	// rotate axis
	modelx_ += x_rad;
	modely_ += y_rad;
	modelz_ += z_rad;
	// load identity
	model_ = glm::mat4(1.0);
	// rotate xyz
	model_ = glm::rotate(model_,  modelx_, glm::vec3( 1.0f, 0.0f, 0.0f ) );
	model_ = glm::rotate(model_,  modely_, glm::vec3( 0.0f, 1.0f, 0.0f ) );
	model_ = glm::rotate(model_,  modelz_, glm::vec3( 0.0f, 0.0f, 1.0f ) );
}

void scene::move_model_far(float distance)
{
	view_ = glm::translate( view_, glm::vec3(0.0f, 0.0f, distance) );
}

void scene::move_model_near(float distance)
{
	view_ = glm::translate( view_, glm::vec3(0.0f, 0.0f, -distance) );
}

glm::mat4 scene::get_mvp()
{
	return model_ * view_ * glm::mat4(1.0f);
}


} // namespace engine
