/*
 *
 * Copyright (c) 2016
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef CONFIG_HPP_INCLUDED
#define CONFIG_HPP_INCLUDED

#if __cplusplus < 201103L
#	error "This library requires at least C++ 11 standard compiler"
#endif // CPP11 detection

#ifdef __GNUC__
#include "config/compiller/gcc.hpp"
#elif defined(_MSC_VER)
#include "config/compiller/ms_visual_cpp.hpp"
#endif // __GNUC__

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#if ( defined(_WIN32) || defined(_WIN64) || defined(_WIN32_WINNT) ) && !defined(__CYGWIN__)
#define __IO_WINDOWS_BACKEND__
#include "win/winconf.hpp"
#endif // defined

#if defined(unix) \
      || defined(__unix) \
      || defined(_XOPEN_SOURCE) \
      || defined(_POSIX_SOURCE) \
      || defined(__CYGWIN__)
#define __IO_POSIX_BACKEND__
#include "posix/posixconf.hpp"
#endif // defined

#include <assert.h>
#include <atomic>
#include <functional>
#include <memory>
#include <system_error>

#ifndef IO_HAS_BOOST
#include "config/libs/intrusive_ptr.hpp"
#else
#include <boost/intrusive_ptr.hpp>
#endif // HAS_BOOST

#include "config/libs/exceptions.hpp"
#include "config/libs/h_allocator.hpp"

#ifndef DECLARE_IPTR
#define DECLARE_IPTR(T) typedef boost::intrusive_ptr<T> s_##T
#endif // DECLARE_IPTR

#endif // CONFIG_HPP_INCLUDED
