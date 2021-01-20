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
#ifndef __IO_WINCONG_HPP_INCLUDED__
#define __IO_WINCONG_HPP_INCLUDED__

#ifndef _WIN32_WINNT_WIN7
#	define _WIN32_WINNT_WIN7                 0x0601 // Windows 7
#endif // _WIN32_WINNT_VISTA

// Check for the default value, for MinGW is Windows XP
#if defined(_WIN32_WINNT) && defined(IO_BUILD)

#	if _WIN32_WINNT < _WIN32_WINNT_WIN7
#		undef  _WIN32_WINNT
#		define WINVER _WIN32_WINNT_WIN7
#		define _WIN32_WINNT _WIN32_WINNT_WIN7
#	endif // _WIN32_WINNT

#	define WIN32_LEAN_AND_MEAN

#elif !defined(_WIN32_WINNT)

#	define WINVER _WIN32_WINNT_WIN7
#	define _WIN32_WINNT _WIN32_WINNT_WIN7

#elif ( _WIN32_WINNT < _WIN32_WINNT_WIN7 )

#	error "Windows 7 is minimal supported Windows version, \
			you mind fogot to define _WIN32_WINNT when on MinGW  \
			or check your winver.h in visual studio project "

#endif // defined

#ifdef WINVER
#	undef WINVER
#	define WINVER _WIN32_WINNT
#endif // WINVER

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>

typedef ::HANDLE os_descriptor_t;

inline os_descriptor_t socket_id(::SOCKET socket) noexcept {
	return reinterpret_cast<::HANDLE>(socket);
}

// shared library
#if defined(IO_SHARED_LIB)
#	if defined(IO_BUILD)
#		ifdef __GNUG__
#			define IO_PUBLIC_SYMBOL __attribute__ ((dllexport))
#			define IO_MALLOC_ATTR __attribute__ ((dllexport,malloc))
#		else
#			define IO_PUBLIC_SYMBOL __declspec(dllexport)
#		endif
#	else
#		ifdef __GNUG__
#			define IO_PUBLIC_SYMBOL __attribute__ ((dllimport))
#			define IO_MALLOC_ATTR __attribute__ ((dllimport,malloc))
#		else
#			define IO_PUBLIC_SYMBOL __declspec(dllimport)
#		endif
#	endif // defined
// static library
#else
#	ifdef __GNUG__
#		define IO_PUBLIC_SYMBOL
#		define IO_MALLOC_ATTR __attribute__((malloc))
#	else
#		define IO_PUBLIC_SYMBOL
#		define IO_MALLOC_ATTR
#	endif // __GNUG__
#endif // IO_SHARED_LIB

#include "memory_traits.hpp"

#endif // __IO_WINCONG_HPP_INCLUDED__
