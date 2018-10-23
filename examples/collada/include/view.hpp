#ifndef __VIEW_HPP_INCLUDED__
#define __VIEW_HPP_INCLUDED__

#include <object.hpp>
#include <cstring>
#include <cstdlib>
#include <cmath>

#define GLFW_INCLUDE_GLEXT 1
#include <GLFW/glfw3.h>

#include "scene.hpp"

namespace engine  {

class model {
public:
	virtual draw(const scene& scn) const = 0;
	virtual ~model() = default;
};

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
	int width_;
	int height_;
};

} // engine

#endif // __VIEW_HPP_INCLUDED__
