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
#ifndef ___STDAFX_HPP__INCLUDED__
#define ___STDAFX_HPP__INCLUDED__

/* C standard library */
#include <cstring>
#include <cmath>
#include <stdexcept>
#include <string>

/* C++ Standard library */
#include <vector>
#include <type_traits>
#include <unordered_map>

/* IO */
#include <core/object.hpp>
#include <core/buffer.hpp>
#include <core/conststring.hpp>

/* OpenGL specific*/
#ifdef _OPENMP
#	include <omp.h>
#endif
#include "openglload.hpp"
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

// data-types
#include "collada.hpp"

#endif // ___STDAFX_HPP__INCLUDED__
