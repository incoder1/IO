#ifndef __VIEW_HPP_INCLUDED__
#define __VIEW_HPP_INCLUDED__

#include "openglload.hpp"
#include "model.hpp"

namespace engine  {

class frame_view {
	frame_view(const frame_view&&) = delete;
	frame_view& operator=(const frame_view&&) = delete;
public:
	frame_view(unsigned int widht, unsigned int height,const char* title);
	~frame_view() noexcept;
	void show(const model* md);

	/* TODO: replace with model when ready */
	gl::s_program initialize_GLSL();


private:
	::GLFWwindow *frame_;
	scene scn_;
	int width_;
	int height_;
	int mouse_prev_x_;
	int mouse_prev_y_;
	double angle_x_;
	double angle_y_;
	double zoom_;
};

} // engine

#endif // __VIEW_HPP_INCLUDED__
