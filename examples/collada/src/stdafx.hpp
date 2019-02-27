#ifndef ___STDAFX_HPP__INCLUDED__
#define ___STDAFX_HPP__INCLUDED__

/* C standard library */
#include <cstring>
#include <cmath>
#include <stdexcept>
#include <string>

/* C++ Standard library */
#include <atomic>
#include <vector>
#include <type_traits>
#include <unordered_map>

/* IO */
#include <object.hpp>
#include <buffer.hpp>
#include <conststring.hpp>
#include <stringpool.hpp>

/* OpenGL specific*/
#ifdef _OPENMP
#	include <omp.h>
#endif
#include "openglload.hpp"
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

#endif // ___STDAFX_HPP__INCLUDED__
