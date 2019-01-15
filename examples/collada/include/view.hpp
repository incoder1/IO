#ifndef __VIEW_HPP_INCLUDED__
#define __VIEW_HPP_INCLUDED__

#include "openglload.hpp"
#include "model.hpp"

namespace engine  {

/// Frame window OpenGL (modern 3.1+ ) based view
/// OpenGL 4.2+ required to support NURBS since tesseleation program used
class frame_view {
	frame_view(const frame_view&&) = delete;
	frame_view& operator=(const frame_view&&) = delete;
public:
	frame_view(unsigned int widht, unsigned int height,const char* title);
	~frame_view() noexcept;
	void show(const s_model& md);
private:
	::GLFWwindow *frame_;
	scene scn_;
	int mouse_prev_x_;
	int mouse_prev_y_;
	float angle_x_;
	float angle_y_;
	float zoom_;
};

} // engine

#endif // __VIEW_HPP_INCLUDED__
