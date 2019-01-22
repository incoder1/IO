#ifndef __OPENGL_LOAD_HPP_INCLUDED__
#define __OPENGL_LOAD_HPP_INCLUDED__

#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
#   include <GL/glew.h>
#   include <GL/wglew.h>
#   define WGL_WINDOWS 1
#   define GLFW_INCLUDE_NONE 1
#endif // _WIN32

#if ( defined(unix)\
      || defined(__unix)\
      || defined(_XOPEN_SOURCE)\
      || defined(_POSIX_SOURCE)\
      || defined(__linux)\
      || defined(__linux__)\
      )\
      && !defined(__APPLE__)
//#   define GL_GLEXT_PROTOTYPES 1
//#   define GLFW_INCLUDE_GLEXT 1
#   include "glad.h"
#   define GLX_UNIX 1
#	define GLFW_INCLUDE_GLEXT 1
#   define GLFW_INCLUDE_NONE 1
#endif // UNIX not MacOS X

#ifdef __APPLE__
#	define GLFW_INCLUDE_GLEXT 1
#	define GLFW_INCLUDE_GLCOREARB 1
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

// force avx2 even for debug
#if defined(__GNUG__)
#	ifdef __AVX2__
#		define GLM_FORCE_AVX2
#	endif // __AVX2__
#	ifdef __AVX__
#		define GLM_FORCE_AVX
#	endif // __AVX__
#	ifdef __SSE4_2__
#		define GLM_FORCE_SSE42
#	endif
#	ifdef __SSE3__
#		define GLM_FORCE_SSE3
#	endif // __SSE3__
#	ifdef __SSE2__
#		define GLM_FORCE_SSE2
#	endif
#endif // defined

#include <stdexcept>
#include <string>

inline void validate_opengl(const char* errmsg) {
	::GLint errc = ::glGetError();
	if( GL_NO_ERROR != errc )
		throw std::runtime_error( std::string(errmsg).append(" error code: ").append(std::to_string(errc) ) );
}

#endif // __OPENGL_LOAD_HPP_INCLUDED__
