#ifndef __IO_WINCONG_HPP_INCLUDED__
#define __IO_WINCONG_HPP_INCLUDED__

#if defined(_WIN32_WINNT) && defined(IO_BUILD)
#	undef _WIN32_WINNT
#	define _WIN32_WINNT 0x0600
#elif !defined(_WIN32_WINNT)
#	define _WIN32_WINNT 0x0600
#elif ( _WIN32_WINNT < 0x0600 )
#	error "Windows Vista is minimal supported windows version"
#endif // defined

#include <winsock2.h>
#include <windows.h>

// shared library
#if defined(IO_SHARED_LIB)
#	if defined(IO_BUILD)
#		ifdef __GNUC__
#			define IO_PUBLIC_SYMBOL __attribute__ ((dllexport))
#		else
#			define IO_PUBLIC_SYMBOL __declspec(dllexport)
#		endif
#	else
#		ifdef __GNUC__
#			define IO_PUBLIC_SYMBOL __attribute__ ((dllimport))
#		else
#			define IO_PUBLIC_SYMBOL __declspec(dllimport)
#		endif
#	endif // defined
// static library
#else
#	define IO_PUBLIC_SYMBOL
#endif // IO_SHARED_LIB

#include "hmalloc.hpp"

#endif // __IO_WINCONG_HPP_INCLUDED__
