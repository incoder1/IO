#ifndef __OPENGL_LOAD_HPP_INCLUDED__
#define __OPENGL_LOAD_HPP_INCLUDED__

#include <GL/glew.h>

#if defined(_WIN32) || defined(__CYGWIN__)
#include <GL/wglew.h>
#endif // _WIN32

#if defined(unix) \
      || defined(__unix) \
      || defined(_XOPEN_SOURCE) \
      || defined(_POSIX_SOURCE) \
      && !defined(__APPLE__)
#include <GL/glxew.h>
#endif // UNIX not MacOS X

#ifdef __APPLE__
#define GLFW_INCLUDE_GLEXT 1
#endif // __APPLE__

#include <GLFW/glfw3.h>

#endif // __OPENGL_LOAD_HPP_INCLUDED__
