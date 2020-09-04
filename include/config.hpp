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
#ifndef IO_CONFIG_HPP_INCLUDED
#define IO_CONFIG_HPP_INCLUDED


#ifdef __GNUG__
#	include "config/compiler/gcc.hpp"
#elif defined(_MSC_VER)
#	include "config/compiler/ms_visual_cpp.hpp"
#endif // __GNUG__

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <assert.h>
#include <atomic>
#include <functional>
#include <memory>
#include <system_error>

#if ( defined(_WIN32) || defined(_WIN64) ) && ! ( defined(__CYGWIN__) || defined(MSYS2) )
#    define __IO_WINDOWS_BACKEND__
#    include "win/winconf.hpp"
#elif defined(unix) \
      || defined(__unix) \
      || defined(_XOPEN_SOURCE) \
      || defined(_POSIX_SOURCE) \
      || defined(__CYGWIN__)
#    define __IO_POSIX_BACKEND__
#    include "posix/posixconf.hpp"
#endif // defined

#ifndef IO_HAS_BOOST
#include "config/libs/intrusive_ptr.hpp"
#else
#include <boost/intrusive_ptr.hpp>
#endif // HAS_BOOST

#include "config/libs/exceptions.hpp"

#ifndef DECLARE_IPTR
#define DECLARE_IPTR(T) typedef boost::intrusive_ptr<T> s_##T
#endif // DECLARE_IPTR

#endif // IO_CONFIG_HPP_INCLUDED
