/*
 *
 * Copyright (c) 2016-2020
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef IO_CONFIG_HPP_INCLUDED
#define IO_CONFIG_HPP_INCLUDED


#if defined(__GNUG__) && !defined(__clang__)
#	include "config/compiler/gcc.hpp"
#elif defined(__clang__)
#	include "config/compiler/clang.hpp"
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

#include "config/libs/exceptions.hpp"
#include "config/libs/h_allocator.hpp"

#ifndef IO_HAS_BOOST
#   include "config/libs/intrusive_ptr.hpp"
#else
#   include <boost/intrusive_ptr.hpp>
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

#endif // IO_CONFIG_HPP_INCLUDED
