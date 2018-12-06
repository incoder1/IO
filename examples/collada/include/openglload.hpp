#ifndef __OPENGL_LOAD_HPP_INCLUDED__
#define __OPENGL_LOAD_HPP_INCLUDED__

#if defined(_WIN32) || defined(__CYGWIN__)
#include <GL/glew.h>
#include <GL/wglew.h>
#define GLFW_INCLUDE_NONE
#endif // _WIN32

#if defined(unix) \
      || defined(__unix) \
      || defined(_XOPEN_SOURCE) \
      || defined(_POSIX_SOURCE) \
      && !defined(__APPLE__)
#include <GL/glew.h>
#include <GL/glxew.h>
#endif // UNIX not MacOS X

#ifdef __APPLE__
#	define GLFW_INCLUDE_GLCOREARB 1
#	define GLFW_INCLUDE_GLEXT 1
#endif // __APPLE__

#include <GLFW/glfw3.h>

#ifndef GLFW_TRUE
#   define GLFW_TRUE GL_TRUE
#endif // GLFW_TRUE

#ifndef GLFW_FALSE
#   define GLFW_FALSE GL_FALSE
#endif // GLFW_FALSE

#ifdef __MINGW64__
#define GLM_ENABLE_EXPERIMENTAL
#endif // WIN32

#include <stdexcept>
#include <string>

inline void validate_opengl(const char* errmsg) {
	::GLint errc = ::glGetError();
	if( GL_NO_ERROR != errc )
		throw std::runtime_error( std::string(errmsg).append(" error code: ").append(std::to_string(errc) ) );
}

#endif // __OPENGL_LOAD_HPP_INCLUDED__
