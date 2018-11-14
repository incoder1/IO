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
#define GLFW_INCLUDE_GLEXT 1
#endif // __APPLE__

#include <GLFW/glfw3.h>

#include <stdexcept>
#include <string>

inline void validate_opengl(const char* errmsg) {
	::GLint errc = ::glGetError();
	if( GL_NO_ERROR != errc )
		throw std::runtime_error( std::string(errmsg).append(" error code: ").append(std::to_string(errc) ) );
}

#endif // __OPENGL_LOAD_HPP_INCLUDED__
