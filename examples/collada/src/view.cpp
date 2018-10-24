#include "view.hpp"

namespace engine  {


// frame_view
frame_view::frame_view(unsigned int widht, unsigned int height,const char* title):
	frame_(nullptr),
	scn_(1.0f, 1.0f, 2.0f, 20.0f),
	width_(widht),
	height_(height)
{
	::glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_NATIVE_CONTEXT_API);
	::glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
	::glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	::glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

	frame_ = ::glfwCreateWindow(widht, height, title, nullptr, nullptr);
	::glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	::glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	::glfwMakeContextCurrent(frame_);

	::glEnable(GL_DEPTH_TEST);
	::glShadeModel(GL_FLAT);
	::glEnable(GL_CULL_FACE);
	::glfwSwapInterval(1);


	::glEnable(GL_LIGHTING);
	::glEnable(GL_NORMALIZE);
	::glEnable(GL_COLOR_MATERIAL);

	::glDepthFunc(GL_LEQUAL);
	::glShadeModel(GL_SMOOTH);
	::glEnable(GL_LINE_SMOOTH);
	::glEnable(GL_POLYGON_SMOOTH);

	// Nice perspective corrections
	::glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	::glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	::glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
}

frame_view::~frame_view() noexcept
{
	// ::glfwFreeCallbacks(frame_);
	::glfwDestroyWindow(frame_);
}

void frame_view::show(const model* md)
{
	::glfwGetFramebufferSize(frame_, &width_, &height_);
	const ::GLFWvidmode *vidmode = ::glfwGetVideoMode(::glfwGetPrimaryMonitor());
	::glfwSetWindowPos(frame_, (vidmode->width - width_) / 2, (vidmode->height - height_) / 2);
	::glfwShowWindow(frame_);
	while( GLFW_FALSE == ::glfwWindowShouldClose(frame_) ) {
		::glViewport(0, 0, width_, height_);
		::glClear(GL_COLOR_BUFFER_BIT);
		::glClearColor(0.0f, 1.0f, 0.0f, 1.0f);

		// insert model rendering
		if(nullptr != md)
            md->draw(scn_);

		::glfwSwapBuffers(frame_);
		::glfwWaitEvents();
	}
}


} // namespace engine
