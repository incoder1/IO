/*
 *
 * Copyright (c) 2016-2019
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#include "stdafx.hpp"
#include "view.hpp"

#include <iostream>

#ifdef GLX_UNIX
extern "C" int gladLoadGL(void);
#endif

#include <cmath>


namespace engine  {


// frame_view
frame_view::frame_view(unsigned int widht, unsigned int height,const char* title):
	frame_(nullptr),
	scn_( scene::perspective( widht, height, 2.0F, 20.0F) ),
	mouse_prev_x_(0),
	mouse_prev_y_(0),
	angle_x_(0.0F),
	angle_y_(0.0F),
	zoom_(-5.0F)
{

#ifdef _WIN32
	::glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_NATIVE_CONTEXT_API);
#endif // _WIN32

#ifdef __APPLE__
	::glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT,GL_TRUE);
	::glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	::glfwWindowHint(GLFW_SAMPLES, 8);
#endif
	::glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
	::glfwWindowHint(GLFW_CONTEXT_RELEASE_BEHAVIOR, GLFW_RELEASE_BEHAVIOR_FLUSH);
	::glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	::glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	::glfwSetErrorCallback( [](int errc, const char *msg) {
		throw std::runtime_error(msg);
	} );

	frame_ = ::glfwCreateWindow(widht, height, title, nullptr, nullptr);

	::glfwSetCursor(frame_, ::glfwCreateStandardCursor(GLFW_HAND_CURSOR) );

	::glfwSetWindowUserPointer(frame_, this );

	::glfwMakeContextCurrent(frame_);

#ifdef WGL_WINDOWS
	// load extensions
	::glewInit();
#elif defined(GLX_UNIX)
	if( 0 == ::gladLoadGL() )
		throw std::runtime_error("Can not load opengl");
#endif // _WIN32

	::glfwSwapInterval(1);

	// Init OpenGL
	//::glEnable(GL_CULL_FACE);
	//::glCullFace(GL_FRONT);

	::glEnable(GL_DEPTH_TEST);
	::glDepthFunc(GL_LEQUAL);

	//::glShadeModel(GL_FLAT);
	//::glShadeModel(GL_SMOOTH);

	::glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	::glEnable(GL_LIGHTING);
	::glEnable(GL_NORMALIZE);
	::glEnable(GL_COLOR_MATERIAL);
	::glEnable( GL_MULTISAMPLE );

	// set up callbacks

	// to convert mouse offset into radians angles
	static constexpr const float TWO_PI = 3.14159265358979323846 * 2;

	// initialize mouse position in the middle of the window
	const ::GLFWvidmode* mode = ::glfwGetVideoMode( ::glfwGetPrimaryMonitor() );
	mouse_prev_x_ = mode->width / 2;
	mouse_prev_y_ = mode->height / 2;

	// mouse motion, rotate model on mouse movement
	::glfwSetCursorPosCallback(frame_, []
	(::GLFWwindow *wnd,double xpos, double ypos) {
		frame_view *self = static_cast<frame_view*>( ::glfwGetWindowUserPointer(wnd) );
		bool left_pressed = GLFW_PRESS == ::glfwGetMouseButton(wnd, GLFW_MOUSE_BUTTON_1);
		if( left_pressed ) {
			int w, h;
			::glfwGetFramebufferSize(wnd, &w, &h);
			float x_delta = TWO_PI  * float( (xpos - self->mouse_prev_x_) / w );
			float y_delta = TWO_PI  * float( (ypos - self->mouse_prev_y_) / h );
			self->angle_x_ -= y_delta;
			self->angle_y_ += x_delta;
		}
		self->mouse_prev_x_ = xpos;
		self->mouse_prev_y_ = ypos;
	} );

	// mouse scroll, move model to scroll
	::glfwSetScrollCallback(frame_, [] (::GLFWwindow *wnd, double xoffset, double yoffset) {
		frame_view *self = static_cast<frame_view*>( ::glfwGetWindowUserPointer(wnd) );
		self->zoom_ += static_cast<float>( yoffset / 2.0 );
		::glfwPostEmptyEvent();
	});

	// update perspective on window resize
	glfwSetWindowSizeCallback(frame_, [](GLFWwindow* wnd, int w, int h) {
		frame_view *self = static_cast<frame_view*>( ::glfwGetWindowUserPointer(wnd) );
		self->scn_.update_view_perspective( static_cast<float>(w), static_cast<float>(h) );
	});

	// keys
	::glfwSetKeyCallback(frame_, [] (::GLFWwindow *wnd,int key,int scancode,int action,int mods) {
		frame_view *self = static_cast<frame_view*>( ::glfwGetWindowUserPointer(wnd) );
		switch (key) {
		case GLFW_KEY_LEFT:
			self->scn_.move_light(-0.5F, 0.0F, 0.0F);
			break;
		case GLFW_KEY_RIGHT:
			self->scn_.move_light(0.5F, 0.0F, 0.0F);
			break;
		case GLFW_KEY_UP:
			self->scn_.move_light(0.0F, 0.5F, 0.0F);
			break;
		case GLFW_KEY_DOWN:
			self->scn_.move_light(0.0F, -0.5F, 0.0F);
			break;
		case GLFW_KEY_W:
			self->scn_.move_light(0.0F, 0.0F, -0.5F);
			break;
		case GLFW_KEY_S:
			self->scn_.move_light(0.0F, 0.0F, 0.5F);
			break;
		case GLFW_KEY_SPACE:
			if (GLFW_RELEASE == action) {
				light_t l = self->scn_.light();
				float w = (0.0F == l.pads[3]) ? 1.0F : 0.0F;
				self->scn_.set_light(l.pads[0], l.pads[1], l.pads[2], w);
			}
			break;
		case GLFW_KEY_ESCAPE:
			if (GLFW_RELEASE == action)
				glfwSetWindowShouldClose(wnd, true);
			break;
		}
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

	constexpr ::GLbitfield CLEAR_FLAGS = GL_ACCUM_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;

	// run loop
	while( GLFW_FALSE == ::glfwWindowShouldClose(frame_) ) {

		scn_.move_model(zoom_);
		scn_.rotate_model(angle_x_, angle_y_);

		::glClear( CLEAR_FLAGS );
		::glClearDepth(1.0F);
		::glfwGetFramebufferSize(frame_, &w, &h);
		::glViewport(0, 0, w, h);
		::glClearColor(1.0F, 1.0F, 1.0F, 1.0F);

		if(md)
			md->render( scn_ );

		::glfwSwapBuffers(frame_);
		::glfwWaitEvents();
	}

}


} // namespace engine
