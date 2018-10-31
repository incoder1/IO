#include "view.hpp"

namespace engine  {

// FIXME: Test code remove this
static const char* VERTEX_SHADER = "\
#version 140\n\
#pragma optimize(on)\n\
precision highp float;\n\
invariant gl_Position;\n\
uniform mat4 mvpMat;\n\
uniform mat4 modelViewMat;\n\
uniform mat4 normalMat;\n\
in vec3 vertexCoord;\n\
in vec3 vertexNormal;\n\
in vec3 vertexColor;\n\
out vec4 outFrontColor;\n\
out vec4 outBackColor;\n\
struct LightInfo {\n\
 vec4 position;\n\
 vec4 ambient;\n\
 vec4 diffuse;\n\
 vec4 specular;\n\
};\n\
struct MaterialInfo {\n\
 vec4 ambient;\n\
 vec4 diffuse;\n\
 vec4 specular;\n\
 vec4 emission;\n\
 float shininess;\n\
};\n\
LightInfo defaultLight() {\n\
	LightInfo result;\n\
	result.position = vec4(0,0,1,0);\n\
	result.ambient = vec4(0,0,0,1);\n\
	result.diffuse = vec4(1,1,1,1);\n\
	result.specular = vec4(1,1,1,1);\n\
	return result;\n\
}\n\
MaterialInfo defaultMaterial() {\n\
	MaterialInfo result;\n\
	result.ambient = vec4(0.2, 0.2, 0.2, 1);\n\
 	result.diffuse = vec4(0.8, 0.8, 0.8, 1);\n\
 	result.specular = vec4(0, 0, 0, 1);\n\
 	result.emission = vec4(0,0,0,1);\n\
 	result.shininess = 0.001;\n\
 	return result;\n\
}\n\
float dot(vec4 lsh, vec4 rhs) {\n\
  return (lsh.x*rhs.x) + (lsh.y*rhs.y) + (lsh.z*rhs.z) + (lsh.w*rhs.w);\n\
}\n\
void getEyeSpace( out vec4 norm, out vec4 position ) {\n\
	norm = normalize( normalMat * vec4(vertexNormal,0) );\n\
	position = modelViewMat * vec4(vertexCoord.xyz,0);\n\
}\n\
vec4 phongModel(LightInfo light, MaterialInfo mat, vec4 position, vec4 norm ) {\n\
	vec4 s = normalize( light.position - position );\n\
	vec4 v = normalize( -position );\n\
	vec4 r = reflect( -s, norm );\n\
	vec4 ambient = light.ambient * mat.ambient;\n\
	float sDotN = max( dot(s,norm), 0.0 );\n\
	vec4 diffuse = light.diffuse * mat.diffuse * sDotN;\n\
	vec4 specular = vec4(0.0);\n\
	if( sDotN > 0.0 ) {\n\
		specular = light.specular * mat.specular * pow( max( dot(r,v), 0.0 ), mat.shininess );\n\
	}\n\
	return ambient + diffuse + specular;\n\
}\n\
void main(void) {\n\
	vec4 eyeNorm;\n\
	vec4 eyePosition;\n\
	getEyeSpace(eyeNorm, eyePosition);\n\
	LightInfo light = defaultLight();\n\
	light.position = vec4(1,1,2,0);\n\
	MaterialInfo mat = defaultMaterial();\n\
	outFrontColor = vec4(vertexColor,1) + phongModel(light, mat, eyePosition, eyeNorm );\n\
	outBackColor = vec4(vertexColor,1) + phongModel(light, mat, eyePosition, -eyeNorm );\n\
	gl_Position = mvpMat * vec4(vertexCoord,1.0);\n\
}";

// TODO: Remove this
static const char* FRAGMENT_SHADER = "\
#version 140 \n\
#pragma optimize(on)\n\
precision highp float;\n\
in vec4 outFrontColor;\n\
in vec4 outBackColor;\n\
out vec4 outFragColor;\n\
void main(void) {\n\
	if( gl_FrontFacing ) {\n\
		outFragColor = outFrontColor;\n\
	} else {\n\
		outFragColor = outBackColor;\n\
	}\n\
}";


static const float VERTEX[216] = {
	// Top Quad
	1.0F, 1.0F, -1.0F,	0.5F, 0.5f, 0.5f,	0.0F, 1.0F, 0.0F,
	-1.0F, 1.0F, -1.0F,	0.5f, 0.5f, 0.5f,	0.0F, 1.0F, 0.0F,
	-1.0F, 1.0F, 1.0F,	0.5f, 0.5f, 0.5f,	0.0F, 1.0F, 0.0F,
	1.0F, 1.0F, 1.0F,	0.5f, 0.5f, 0.5f,	0.0F, 1.0F, 0.0F,

	// Bottom Quad
	1.0F, -1.0F, 1.0F,	0.0F, 1.0F, 0.0F,	 0.0F,-1.0F, 0.0F,
	-1.0F,-1.0F, 1.0F,	0.0F, 1.0F,0.0F,	 0.0F,-1.0F, 0.0F,
	-1.0F,-1.0F,-1.0F,	0.0F, 1.0F, 0.0F,	 0.0F,-1.0F, 0.0F,
	1.0F,-1.0F,-1.0F,	0.0F, 1.0F, 0.0F,	 0.0F,-1.0F, 0.0F,
	// front Quad
	1.0F, 1.0F, 1.0F,	1.0F, 0.0F, 0.0F,	0.0F, 0.0F, 1.0F,
	-1.0F, 1.0F, 1.0F,	1.0F, 0.0F, 0.0F,	0.0F, 0.0F, 1.0F,
	-1.0F, -1.0F, 1.0F,	1.0F, 0.0F, 0.0F,	0.0F, 0.0F, 1.0F,
	1.0F, -1.0F, 1.0F,	1.0F, 0.0F, 0.0F,	0.0F, 0.0F, 1.0F,
	// Back Quad
	1.0F, -1.0F, -1.0F,	1.0F, 1.0F, 0.0F,	0.0F, 0.0F, -1.0F,
	-1.0F, -1.0F, -1.0F,	1.0F, 1.0F, 0.0F,	0.0F, 0.0F, -1.0F,
	-1.0F,  1.0F, -1.0F,	1.0F, 1.0F, 0.0F,	0.0F, 0.0F, -1.0F,
	1.0F,  1.0F, -1.0F,	1.0F, 1.0F, 0.0F,	0.0F, 0.0F, -1.0F,
	// Left Quad
	-1.0F, 1.0F, 1.0F,	0.0F, 0.0F, 1.0F,	-1.0F, 0.0F, 0.0F,
	-1.0F, 1.0F,-1.0F,	0.0F, 0.0F, 1.0F,	-1.0F, 0.0F, 0.0F,
	-1.0F,-1.0F,-1.0F,	0.0F, 0.0F,	1.0F,	-1.0F, 0.0F, 0.0F,
	-1.0F,-1.0F, 1.0F,	0.0F, 0.0F, 1.0F,	-1.0F, 0.0F, 0.0F,
	// Right Quad
	1.0F, 1.0F,-1.0F,	1.0F, 0.0F, 1.0F,	1.0F, 0.0F, 0.0F,
	1.0F, 1.0F, 1.0F,	1.0F, 0.0F, 1.0F,	1.0F, 0.0F, 0.0F,
	1.0F,-1.0F, 1.0F,	1.0F, 0.0F, 1.0F,	1.0F, 0.0F, 0.0F,
	1.0F,-1.0F,-1.0F,	1.0F, 0.0F, 1.0F,	1.0F, 0.0F, 0.0F
};

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

// FIMXE: remove test code
static gl::s_program initialize_GLSL(const gl::s_buffer& vbo)
{

	gl::shader vertex_sh(gl::shader_type::vertex, VERTEX_SHADER );
	gl::shader fragment_sh(gl::shader_type::fragment, FRAGMENT_SHADER );
	gl::s_program ret =	gl::program::create( std::move(vertex_sh), std::move(fragment_sh) );

	ret->bind_attrib_location(0, "vertexCoord");
	ret->bind_attrib_location(1, "vertexColor");
	ret->bind_attrib_location(2, "vertexNormal");

	ret->pass_vertex_attrib_array(0, vbo, false, 9, 3, 0);
	ret->pass_vertex_attrib_array(1, vbo, false, 9, 3, 3);
	ret->pass_vertex_attrib_array(2, vbo, false, 9, 3, 6);

	ret->link();

	return ret;
}

void frame_view::show(const model* md)
{

	// show window
	int w, h;
	::glfwGetFramebufferSize(frame_, &w, &h);
	const ::GLFWvidmode *vidmode = ::glfwGetVideoMode(::glfwGetPrimaryMonitor());
	// move in the middle of the screen
	::glfwSetWindowPos(frame_, (vidmode->width - w) / 2, (vidmode->height - h) / 2);
	::glfwShowWindow(frame_);

	// FIXME: Test code to remove

	gl::s_buffer vertex = gl::buffer::create( VERTEX, sizeof(VERTEX),
						  gl::buffer_type::ARRAY_BUFFER,
						  gl::data_type::FLOAT,
						  gl::buffer_usage::STATIC_DRAW
											);

	// FIXME: test code to remove
	gl::s_program prg = initialize_GLSL(vertex);

	const ::GLint mvpUL = prg->uniform_location("mvpMat");
	const ::GLint modelVeiwMatUL = prg->uniform_location("modelViewMat");
	const ::GLint normalMatUL = prg->uniform_location("normalMat");

	::glm::mat4 projection_mat;
	::glm::mat4 model_view_mat;

	// run loop
	while( GLFW_FALSE == ::glfwWindowShouldClose(frame_) ) {

		scn_.move_model(zoom_);
		scn_.rotate_model(angle_x_, angle_y_);
		scn_.get_matrix(projection_mat,model_view_mat);


		::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		::glClearDepth(1.0f);
		::glfwGetFramebufferSize(frame_, &w, &h);
		::glViewport(0, 0, w, h);
		::glClearColor(1.0F, 1.0F, 1.0F, 1.0F);

		// FIXME: Test code to be removed
		prg->start();
		::glUniformMatrix4fv(mvpUL, 1, GL_FALSE, glm::value_ptr( projection_mat * model_view_mat ) );
		::glUniformMatrix4fv(modelVeiwMatUL, 1, GL_FALSE, glm::value_ptr( model_view_mat ) );
		::glUniformMatrix4fv(normalMatUL, 1, GL_FALSE, glm::value_ptr( model_view_mat ) );
		::glDrawArrays(GL_QUADS, 0, 24);
		prg->stop();

		// FIXME: insert model rendering
		//   md->draw(scn_);

		::glfwSwapBuffers(frame_);
		::glfwWaitEvents();
	}

}


} // namespace engine
