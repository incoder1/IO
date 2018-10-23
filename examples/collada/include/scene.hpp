#ifndef SCENE_HPP_INCLUDED
#define SCENE_HPP_INCLUDED

#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace engine  {

class scene {
public:
	scene(float width, float height,float eye_distance,float depth);

	void rotate_model(float x_rad, float y_rad, float z_rad);

	void move_model_far(float distance);

	void move_model_near(float distance);

	glm::mat4 get_mvp();

private:
	glm::mat4 model_;
	glm::mat4 world_;
	glm::vec3 light_position_;

	float modelx_;
	float modely_;
	float modelz_;
};

} // namespace engine

#endif // SCENE_HPP_INCLUDED
