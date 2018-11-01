#include "view.hpp"

namespace engine  {


// frame_view
frame_view::frame_view(unsigned int widht, unsigned int height,const char* title):
	frame_(nullptr),
	scn_(1.0F, 1.0F, 2.0F, 20.0F),
	mouse_prev_x_(0),
	mouse_prev_y_(0),
	angle_x_(0.0F),
	angle_y_(0.0F),
	zoom_(-5.0F)
{
	::glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_NATIVE_CONTEXT_API);
	::glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
	::glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	::glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	::glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_FALSE);
	::glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	::glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	::glfwWindowHint(GLFW_SAMPLES, 8);
	frame_ = ::glfwCreateWindow(widht, height, title, nullptr, nullptr);
	if(nullptr == frame_)
		throw std::bad_alloc();

	::glfwSetWindowUserPointer(frame_, this );

	::glfwMakeContextCurrent(frame_);
	// load extensions
	::glewInit();
	::glfwSwapInterval(1);

	::glShadeModel(GL_FLAT);
	::glEnable(GL_CULL_FACE);
	::glEnable(GL_DEPTH_TEST);
	::glDepthFunc(GL_LEQUAL);
	::glShadeModel(GL_SMOOTH);
	::glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	::glEnable(GL_LIGHTING);
	::glEnable(GL_NORMALIZE);
	::glEnable(GL_COLOR_MATERIAL);
	::glEnable( GL_MULTISAMPLE );

	// set up callbacks

	// mouse motion
	::glfwSetCursorPosCallback(frame_, []
	(::GLFWwindow *wnd,double xpos, double ypos) {
		frame_view *self = static_cast<frame_view*>( ::glfwGetWindowUserPointer(wnd) );
		int w, h;
		::glfwGetFramebufferSize(wnd, &w, &h);
		float x_delta = (glm::pi<float>() * 2)  * float( (xpos - self->mouse_prev_x_) / w );
		float y_delta = (glm::pi<float>() * 2)  * float( (ypos - self->mouse_prev_y_) / h );
		self->angle_x_ -= y_delta;
		self->angle_y_ += x_delta;
		self->mouse_prev_x_ = xpos;
		self->mouse_prev_y_ = ypos;
	} );

	// mouse scroll
	::glfwSetScrollCallback(frame_, [] (::GLFWwindow *wnd, double xoffset, double yoffset) {
		frame_view *self = static_cast<frame_view*>( ::glfwGetWindowUserPointer(wnd) );
		self->zoom_ += static_cast<float>(yoffset);
		::glfwPostEmptyEvent();
	});
}

frame_view::~frame_view() noexcept
{
	::glfwDestroyWindow(frame_);
}

void frame_view::show(const s_model& md)
{

	// show window
	int w, h;
	::glfwGetFramebufferSize(frame_, &w, &h);
	const ::GLFWvidmode *vidmode = ::glfwGetVideoMode(::glfwGetPrimaryMonitor());
	// move in the middle of the screen
	::glfwSetWindowPos(frame_, (vidmode->width - w) / 2, (vidmode->height - h) / 2);
	::glfwShowWindow(frame_);

	// run loop
	while( GLFW_FALSE == ::glfwWindowShouldClose(frame_) ) {

		scn_.move_model(zoom_);
		scn_.rotate_model(angle_x_, angle_y_);

		::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		::glClearDepth(1.0f);
		::glfwGetFramebufferSize(frame_, &w, &h);
		::glViewport(0, 0, w, h);
		::glClearColor(1.0F, 1.0F, 1.0F, 1.0F);

		if(md)
		 	md->draw(scn_);

		::glfwSwapBuffers(frame_);
		::glfwWaitEvents();
	}

}


} // namespace engine
