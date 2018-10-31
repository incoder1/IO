#ifndef __VIEW_HPP_INCLUDED__
#define __VIEW_HPP_INCLUDED__

#include "openglload.hpp"
#include "model.hpp"

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif // _USE_MATH_DEFINES

#include <cmath>

namespace engine  {

class frame_view {
	frame_view(const frame_view&&) = delete;
	frame_view& operator=(const frame_view&&) = delete;
public:
	frame_view(unsigned int widht, unsigned int height,const char* title);
	~frame_view() noexcept;
	void show(const model* md);

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
