/*
 *
 * Copyright (c) 2016-2023
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef __LIB_IO_CONFIG_HPP_INCLUDED__
#define __LIB_IO_CONFIG_HPP_INCLUDED__

// Compiler specific configuration selection
// e.g. intrinsics, libc hints etc
#if defined(__GNUG__) && !defined(__clang__)
#	include "compiler/gcc.hpp"
#elif defined(__clang__)
#	include "compiler/clang.hpp"
#elif defined(_MSC_VER)
#	include "compiler/ms_visual_cpp.hpp"
#endif // __GNUG__

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

// Operating system specific configuration, i.e. OS depended syscall headers etc
#if ( defined(_WIN32) || defined(_WIN64) ) && ! ( defined(__CYGWIN__) || defined(MSYS2) )
#	define __IO_WINDOWS_BACKEND__
#	include "win/winconf.hpp"
#elif		defined(unix) \
		||	defined(__unix) \
		||	defined(_XOPEN_SOURCE) \
		||	defined(_POSIX_SOURCE) \
		||	defined(__CYGWIN__)
#	define __IO_POSIX_BACKEND__
#	include "posix/posixconf.hpp"
#endif // defined

#include <assert.h>
#include <atomic>
#include <functional>
#include <memory>
#include <system_error>

#include "io/config/libs/exceptions.hpp"

#ifndef IO_HAS_BOOST
#	include "io/config/libs/intrusive_ptr.hpp"
#else
#	include <boost/intrusive_ptr.hpp>
#endif // HAS_BOOST

#ifndef DECLARE_IPTR
#	define DECLARE_IPTR(T) typedef boost::intrusive_ptr<T> s_##T
#endif // DECLARE_IPTR

#ifndef __HAS_CPP_14
namespace std {

	// C++11 version of std::exchange for internal use.
	template <typename _Tp, typename _Up = _Tp>
	inline _Tp exchange(_Tp& __obj, _Up&& __new_val)
	{
		_Tp __old_val = std::move(__obj);
		__obj = std::forward<_Up>(__new_val);
		return __old_val;
	}

} // namespace std
#endif // __HAS_CPP_14

#endif // __LIB_IO_CONFIG_HPP_INCLUDED__
