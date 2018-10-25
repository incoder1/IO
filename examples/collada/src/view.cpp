#include "view.hpp"

#include <cmath>
#include <new>

namespace engine  {

// TODO: Remove this
static const char* VERTEX_SHADER = "\
#version 140 \n \
#pragma optimize(on) \n \
precision highp float; \n \
invariant gl_Position; \
uniform mat4 mvpMat; \
uniform mat4 modelViewMat; \
uniform mat4 normalMat; \
uniform vec3 matAmb; \
uniform vec3 matDiff; \
uniform vec3 matSpec; \
uniform float matShininess; \
uniform float matDissolve; \
uniform vec3 lightPosition; \
in vec3 vertexCoord; \
in vec3 vertexNormal; \
out vec4 outFrontColor; \
out vec4 outBackColor; \
struct LightInfo { \
 vec4 position; \
 vec4 ambient; \
 vec4 diffuse; \
 vec4 specular; \
}; \
struct MaterialInfo { \
 vec4 ambient; \
 vec4 diffuse; \
 vec4 specular; \
 vec4 emission; \
 float shininess; \
}; \
LightInfo defaultLight() { \
	LightInfo result; \
	result.position = vec4(0,0,1,0); \
	result.ambient = vec4(0,0,0,1); \
	result.diffuse = vec4(1,1,1,1); \
	result.specular = vec4(1,1,1,1); \
	return result; \
} \
MaterialInfo defaultMaterial() { \
	MaterialInfo result; \
	result.ambient = vec4(0.2, 0.2, 0.2, 1); \
 	result.diffuse = vec4(0.8, 0.8, 0.8, 1); \
 	result.specular = vec4(0, 0, 0, 1); \
 	result.emission = vec4(0,0,0,1); \
 	result.shininess = 0.001; \
 	return result; \
} \
float dot(vec4 lsh, vec4 rhs) { \
	return (lsh.x*rhs.x) + (lsh.y*rhs.y) + (lsh.z*rhs.z) + (lsh.w*rhs.w); \
} \
void getEyeSpace( out vec4 norm, out vec4 position ) { \
	norm = normalize( normalMat * vec4(vertexNormal,0) ); \
	position = modelViewMat * vec4(vertexCoord.xyz,0); \
} \
vec4 phongModel(LightInfo light, MaterialInfo mat, vec4 position, vec4 norm ) { \
	vec4 s = normalize( light.position - position ); \
	vec4 v = normalize( -position ); \
	vec4 r = reflect( -s, norm ); \
	vec4 ambient = light.ambient * mat.ambient; \
	float sDotN = max( dot(s,norm), 0.0 ); \
	vec4 diffuse = light.diffuse * mat.diffuse * sDotN; \
	vec4 specular = vec4(0.0); \
	if( sDotN > 0.0 ) { \
		float shininess = mat.shininess; \
		if(0.0 == shininess) { \
			shininess = 4; \
		} \
		specular = light.specular * mat.specular * pow( max( dot(r,v), 0.0 ), shininess ); \
	} \
	return ambient + diffuse + specular; \
} \
void main(void) { \
	vec4 eyeNorm; \
	vec4 eyePosition; \
	getEyeSpace(eyeNorm, eyePosition); \
	LightInfo light = defaultLight(); \
	light.position = vec4( lightPosition, 0); \
	MaterialInfo mat = defaultMaterial(); \
	mat.ambient = vec4(matAmb,matDissolve); \
	mat.diffuse = vec4(matDiff,matDissolve); \
	mat.specular = vec4(matSpec,matDissolve); \
	mat.shininess = matShininess; \
	outFrontColor = phongModel(light, mat, eyePosition, eyeNorm ); \
	outBackColor = phongModel(light, mat, eyePosition, -eyeNorm ); \
	gl_Position = mvpMat * vec4(vertexCoord,1.0); \
}";

// TODO: Remove this
static const char* FRAGMENT_SHADER = "\
#version 140 \n \
#pragma optimize(on) \n \
precision highp float; \n \
in vec4 outFrontColor; \
in vec4 outBackColor; \
const vec4 GAMMA = vec4( 1.0 / 2.2 ); \
out vec4 outFragColor; \
void main(void) { \
	if( gl_FrontFacing ) { \
		outFragColor = pow(outFrontColor, GAMMA); \
	} else { \
		outFragColor = pow( outBackColor, GAMMA); \
	} \
}";

// TODO remove this
static const float __QUAD_VERTEX[72] =
		{// Top Quad
			1.0F,  1.0F, -1.0F,
			-1.0F, 1.0F, -1.0F,
			-1.0F,  1.0F, 1.0F,
			1.0F,  1.0F,  1.0F,
			// Bottom Quad
			 1.0F, -1.0F, 1.0F,
			 -1.0F, -1.0F,  1.0F,
			 -1.0F, -1.0F, -1.0F,
			 1.0F, -1.0F, -1.0F,
			// Front Quad
			 1.0F, 1.0F, 1.0F,
			-1.0F, 1.0F, 1.0F,
			-1.0F, -1.0F, 1.0F,
			 1.0F, -1.0F, 1.0F,
			// Back Quad
			  1.0F, -1.0F, -1.0F,
			 -1.0F, -1.0F, -1.0F,
			 -1.0F,  1.0F, -1.0F,
			  1.0F,  1.0F, -1.0F,
			// Left Quad
			-1.0F, 1.0F, 1.0F,
			-1.0F, 1.0F, -1.0F,
			-1.0F, -1.0F, -1.0F,
			-1.0F, -1.0F, 1.0F,
			// Right Quad
			 1.0F, 1.0F, -1.0F,
			 1.0F, 1.0F,  1.0F,
			 1.0F, -1.0F, 1.0F,
			 1.0F, -1.0F, -1.0F };

static const float __COLORS[18] = {
			// Top colors - Gray
			0.5F, 0.5F, 0.5F,
			// Bottom colors - Green
			0.0F, 1.0F, 0.0F,
			// Front colors - Red
			1.0F, 0.0F, 0.0F,
			// Back colors - Yellow
			1.0F, 1.0F, 0.0F,
			// Left colors - Blue
			0.0F, 0.0F, 1.0F,
			// Right colors - Magenta
			1.0F, 0.0F, 1.0F
};

static const float __NORMAL[18] = {
			// Top
			0.0F, 1.0F, 0.0F,
			// Bottom
			0.0F, -1.0F, 0.0F,
			// Front
			0.0F, 0.0F, 1.0F,
			// Back
			0.0F, 0.0F, -1.0F,
			// Left
			-1.0F, 0.0F, 0.0F,
			// Right
			1.0F, 0.0F, 0.0F
};


std::vector<float> make_vertex_attribs3f(const float* a, std::size_t len)
{
    std::vector<float> ret( len << 2);
    for (unsigned i = 0; i < len; i += 3) {
		for (unsigned j = 0; j < 4; j++) {
			ret.push_back(a[i]);
			ret.push_back(a[i + 1]);
			ret.push_back(a[i + 2]);
		}
    }
    return ret;
}

static constexpr double MATH_PI = 3.14159265358979323846;

// frame_view
frame_view::frame_view(unsigned int widht, unsigned int height,const char* title):
	frame_(nullptr),
	scn_(1.0F, 1.0F, 2.0F, 20.0F),
	width_(widht),
	height_(height),
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
	::glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	::glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	frame_ = ::glfwCreateWindow(widht, height, title, nullptr, nullptr);
	if(nullptr == frame_)
		throw std::bad_alloc();

	::glfwSetWindowUserPointer(frame_, this );

	::glfwMakeContextCurrent(frame_);
	// load extensions
    ::glewInit();


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
	ret->gen_vertex_attrib_arrays(3);
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
			double y_delta = MATH_PI * (double(ypos - self->mouse_prev_x_) / h);
			self->angle_x_ += y_delta;
			double x_delta = MATH_PI * (double(xpos - self->mouse_prev_y_) / w);
			self->angle_y_ += x_delta;
	  } );

	// mouse scroll
	::glfwSetScrollCallback(frame_, [] (::GLFWwindow *wnd, double xoffset, double yoffset) {
		frame_view *self = static_cast<frame_view*>( ::glfwGetWindowUserPointer(wnd) );
		self->zoom_ += yoffset;
		::glfwPostEmptyEvent();
	});

	// show window
	::glfwGetFramebufferSize(frame_, &width_, &height_);
	const ::GLFWvidmode *vidmode = ::glfwGetVideoMode(::glfwGetPrimaryMonitor());
	// move in the middle of the screen
	::glfwSetWindowPos(frame_, (vidmode->width - width_) / 2, (vidmode->height - height_) / 2);
	::glfwShowWindow(frame_);


	gl::s_program prg = initialize_GLSL();

	gl::s_buffer vertex = gl::buffer::create( __QUAD_VERTEX, sizeof(__QUAD_VERTEX),
		gl::buffer_type::ARRAY_BUFFER,
		gl::data_type::FLOAT,
		gl::stride::THREE,
        gl::buffer_usage::STATIC_DRAW
	);

	std::vector<float> attr = make_vertex_attribs3f(__COLORS, 18);
	gl::s_buffer colors = gl::buffer::create( &(*attr.begin()),
		attr.size(),
		gl::buffer_type::ARRAY_BUFFER,
		gl::data_type::FLOAT,
		gl::stride::THREE,
        gl::buffer_usage::STATIC_DRAW);

	attr = make_vertex_attribs3f(__NORMAL, 18);
	gl::s_buffer normals = gl::buffer::create(&(*attr.begin()),
		attr.size(),
		gl::buffer_type::ARRAY_BUFFER,
		gl::data_type::FLOAT,
		gl::stride::THREE,
        gl::buffer_usage::STATIC_DRAW);

	 ::GLuint mvpUL = prg->uniform_location("mvpMat");
	 ::GLuint modelVeiwMatUL = prg->uniform_location("modelViewMat");
	 ::GLuint normalMatUL = prg->uniform_location("normalMat");

	// run loop
	while( GLFW_FALSE == ::glfwWindowShouldClose(frame_) ) {
		::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear the
		// Set background depth to farthest
		::glClearDepth(1.0f);
		::glViewport(0, 0, width_, height_);

		::glClearColor(1.0F, 1.0F, 1.0F, 1.0F);



		// insert model rendering
		//if(nullptr != md)
		//   md->draw(scn_);

		::glfwSwapBuffers(frame_);
		::glfwWaitEvents();
	}
}


} // namespace engine
