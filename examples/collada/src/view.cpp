#include "view.hpp"

#include <cmath>
#include <new>

namespace engine  {

// TODO: Remove this
static const char* VERTEX_SHADER = "\
#version 140\n\
#pragma optimize(on)\n\
precision highp float;\
invariant gl_Position;\
uniform mat4 mvpMat;\
in vec3 vertexCoord;\
in vec3 vertexColor;\
in vec3 vertexNormal;\
out vec4 frontColor;\
void main(void) {\
	frontColor =  vec4(0.0, 1.0, 0.0, 1.0); \
	gl_Position = mvpMat * vec4(vertexCoord.xyz,1.0);\
}";

// TODO: Remove this
static const char* FRAGMENT_SHADER = "\
#version 140\n \
#pragma optimize(on) \n \
in vec4 frontColor;\
out vec4 outFragColor;\
void main(void) { outFragColor =  frontColor;}";

// TODO remove this
static const float __QUAD_VERTEX[216] =
		{   /*    coordinate   |    color       |    normal     */
			// Top Quad
			1.0F,  1.0F, -1.0F, 0.5F, 0.5F, 0.5F, 0.0F, 1.0F, 0.0F,
			-1.0F, 1.0F, -1.0F, 0.5F, 0.5F, 0.5F, 0.0F, 1.0F, 0.0F,
			-1.0F,  1.0F, 1.0F, 0.5F, 0.5F, 0.5F, 0.0F, 1.0F, 0.0F,
			1.0F,  1.0F,  1.0F, 0.5F, 0.5F, 0.5F, 0.0F, 1.0F, 0.0F,
			// Bottom Quad
			 1.0F, -1.0F, 1.0F, 0.0F, 1.0F, 0.0F, 0.0F, 1.0F, 0.0F,
			 -1.0F, -1.0F,  1.0F, 0.0F, 1.0F, 0.0F, 0.0F, 1.0F, 0.0F,
			 -1.0F, -1.0F, -1.0F, 0.0F, 1.0F, 0.0F, 0.0F, 1.0F, 0.0F,
			 1.0F, -1.0F, -1.0F, 0.0F, 1.0F, 0.0F, 0.0F, 1.0F, 0.0F,
			// Front Quad
			 1.0F, 1.0F, 1.0F, 1.0F, 0.0F, 0.0F, 0.0F, 0.0F, 1.0F,
			-1.0F, 1.0F, 1.0F, 1.0F, 0.0F, 0.0F, 0.0F, 0.0F, 1.0F,
			-1.0F, -1.0F, 1.0F, 1.0F, 0.0F, 0.0F, 0.0F, 0.0F, 1.0F,
			 1.0F, -1.0F, 1.0F, 1.0F, 0.0F, 0.0F, 0.0F, 0.0F, 1.0F,
			// Back Quad
			  1.0F, -1.0F, -1.0F, 1.0F, 1.0F, 0.0F, 0.0F, 0.0F, -1.0F,
			 -1.0F, -1.0F, -1.0F, 1.0F, 1.0F, 0.0F, 0.0F, 0.0F, -1.0F,
			 -1.0F,  1.0F, -1.0F, 1.0F, 1.0F, 0.0F, 0.0F, 0.0F, -1.0F,
			  1.0F,  1.0F, -1.0F, 1.0F, 1.0F, 0.0F, 0.0F, 0.0F, -1.0F,
			// Left Quad
			-1.0F, 1.0F, 1.0F, 0.0F, 0.0F, 1.0F, -1.0F, 0.0F, 0.0F,
			-1.0F, 1.0F, -1.0F, 0.0F, 0.0F, 1.0F, -1.0F, 0.0F, 0.0F,
			-1.0F, -1.0F, -1.0F, 0.0F, 0.0F, 1.0F, -1.0F, 0.0F, 0.0F,
			-1.0F, -1.0F, 1.0F, 0.0F, 0.0F, 1.0F, -1.0F, 0.0F, 0.0F,
			// Right Quad
			 1.0F, 1.0F, -1.0F, 1.0F, 0.0F, 1.0F, 1.0F, 0.0F, 0.0F,
			 1.0F, 1.0F,  1.0F, 1.0F, 0.0F, 1.0F, 1.0F, 0.0F, 0.0F,
			 1.0F, -1.0F, 1.0F, 1.0F, 0.0F, 1.0F, 1.0F, 0.0F, 0.0F,
			 1.0F, -1.0F, -1.0F, 1.0F, 0.0F, 1.0F, 1.0F, 0.0F, 0.0F};


static constexpr double MATH_PI = 3.14159265358979323846;

// frame_view
frame_view::frame_view(unsigned int widht, unsigned int height,const char* title):
	frame_(nullptr),
	scn_(1.0F, 1.0F, 2.0F, 20.0F),
	mouse_prev_x_(0),
	mouse_prev_y_(0),
	angle_x_(0.0D),
	angle_y_(0.0D),
	zoom_(0.0D)
{
	::glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_NATIVE_CONTEXT_API);
	::glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
	::glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	::glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	::glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_FALSE);
	::glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	::glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	::glfwWindowHint(GLFW_SAMPLES, 8);
	//::glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);
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
}

frame_view::~frame_view() noexcept
{
	::glfwDestroyWindow(frame_);
}

// TODO: remove
gl::s_program frame_view::initialize_GLSL()
{

 	gl::shader vertex(gl::shader_type::vertex, VERTEX_SHADER );
	gl::shader fragment(gl::shader_type::fragment, FRAGMENT_SHADER );
	gl::s_program ret =	gl::program::create( std::move(vertex), std::move(fragment) );
	ret->link();

	// init VAO
	//ret->gen_vertex_attrib_arrays(3);
	ret->bind_attrib_location(0, "vertexCoord");
	ret->bind_attrib_location(1, "vertexColor");
	ret->bind_attrib_location(2, "vertexNormal");

	return ret;
}

void frame_view::show(const model* md)
{
	// set up callbacks

	// mouse motion
	::glfwSetCursorPosCallback(frame_, []
		(::GLFWwindow *wnd,double xpos, double ypos) {
			frame_view *self = static_cast<frame_view*>( ::glfwGetWindowUserPointer(wnd) );
			int w, h;
			::glfwGetFramebufferSize(wnd, &w, &h);
			float x_delta = glm::pi<float>() * float( (xpos - self->mouse_prev_x_) / w );
			float y_delta = glm::pi<float>() * float( (ypos - self->mouse_prev_y_) / h );
			self->angle_x_ += y_delta * 0.1;
			self->angle_y_ += x_delta * 0.1;
			self->mouse_prev_x_ = xpos;
			self->mouse_prev_y_ = ypos;
	  } );

	// mouse scroll
	::glfwSetScrollCallback(frame_, [] (::GLFWwindow *wnd, double xoffset, double yoffset) {
		frame_view *self = static_cast<frame_view*>( ::glfwGetWindowUserPointer(wnd) );
		self->zoom_ += static_cast<float>(yoffset);
		::glfwPostEmptyEvent();
	});

	// show window
	int w, h;
	::glfwGetFramebufferSize(frame_, &w, &h);
	const ::GLFWvidmode *vidmode = ::glfwGetVideoMode(::glfwGetPrimaryMonitor());
	// move in the middle of the screen
	::glfwSetWindowPos(frame_, (vidmode->width - w) / 2, (vidmode->height - h) / 2);
	::glfwShowWindow(frame_);


	gl::s_program prg = initialize_GLSL();

	gl::s_buffer vertex = gl::buffer::create( __QUAD_VERTEX, sizeof(__QUAD_VERTEX),
		gl::buffer_type::ARRAY_BUFFER,
		gl::data_type::FLOAT,
        gl::buffer_usage::STATIC_DRAW
	);

	prg->pass_vertex_attrib_array(0, vertex, false, 3, 9, 0);
	prg->pass_vertex_attrib_array(1, vertex, false, 3, 9, 3);
	prg->pass_vertex_attrib_array(2, vertex, false, 3, 9, 6);


	 ::GLuint mvpUL = prg->uniform_location("mvpMat");
	 //::GLuint modelVeiwMatUL = prg->uniform_location("modelViewMat");
	 //::GLuint normalMatUL = prg->uniform_location("normalMat");


	 //scn_.move_model_far(zoom_);
	 //scn_.rotate_model(angle_x_, angle_y_, 0.0F);

	glm::mat4 progjection = glm::perspective(glm::pi<float>() * 0.25f, 4.0f / 3.0f, 0.1f, 20.f);
	glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3));
	glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));
	//glm::mat4 normal(1.0);

	// run loop
	while( GLFW_FALSE == ::glfwWindowShouldClose(frame_) ) {

		view = glm::rotate(view, angle_x_, glm::vec3(1.0f, 0.0f, 0.0f));
		view = glm::rotate(view, angle_y_, glm::vec3(0.0f, 1.0f, 0.0f));
		view = glm::translate(view, glm::vec3(0.0f, 0.0f, zoom_) );

		glm::mat4 mvp = progjection * view * model;

		::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		::glClearDepth(1.0f);
		::glfwGetFramebufferSize(frame_, &w, &h);
		::glViewport(0, 0, w, h);
		::glClearColor(1.0F, 1.0F, 1.0F, 1.0F);

		prg->start();
			::glUniformMatrix4fv(mvpUL, 1, GL_FALSE, glm::value_ptr(mvp) );
			//::glUniformMatrix4fv(modelVeiwMatUL, 1, GL_FALSE, glm::value_ptr(view) );
			//::glUniformMatrix4fv(normalMatUL, 1, GL_FALSE, glm::value_ptr(normal) );
			::glDrawArrays(GL_QUADS, 0, 24);
		prg->stop();
		if (GL_NO_ERROR != ::glGetError() )
				throw std::runtime_error("Can not initialize GLSL program vertex attributes");

		// insert model rendering
		//if(nullptr != md)
		//   md->draw(scn_);

		::glfwSwapBuffers(frame_);
		::glfwWaitEvents();
	}
}


} // namespace engine
