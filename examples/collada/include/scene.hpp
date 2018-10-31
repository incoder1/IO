#ifndef __SCENE_HPP_INCLUDED__
#define __SCENE_HPP_INCLUDED__

#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace engine  {

class scene {
public:
	scene(float width, float height,float eye_distance,float depth);

	void rotate_model(float x_rad, float y_rad);

	void move_model(float distance);

	void get_matrix(glm::mat4 &prj, glm::mat4& mv) const;

private:

	float width_;
	float height_;
	float eye_distance_;
	float depth_;

	float angle_x_;
	float angle_y_;
	float distance_z_;
};

} // namespace engine

#endif // __SCENE_HPP_INCLUDED__
