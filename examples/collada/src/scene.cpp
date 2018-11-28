#include "stdafx.hpp"
#include "scene.hpp"

// force avx2 even for debug
#if defined(__GNUG__)
#	ifdef __AVX2__
#		define GLM_FORCE_AVX2
#	elif __AVX__
#		define GLM_FORCE_AVX
#	elif __SSE4_2__
#		define GLM_FORCE_SSE42
#	elif __SSE3__
#		define GLM_FORCE_SSE3
#	elif __SSE2__
#		define GLM_FORCE_SSE2
#	endif
#endif // defined

#include <glm/vec3.hpp>
#include <glm/gtx/transform.hpp>

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
