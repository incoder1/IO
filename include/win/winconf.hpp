#ifndef __IO_WINCONG_HPP_INCLUDED__
#define __IO_WINCONG_HPP_INCLUDED__

#ifndef _WIN32_WINNT_VISTA
#	define _WIN32_WINNT_VISTA                  0x0600 // Windows Vista
#endif // _WIN32_WINNT_VISTA

// Check for the default value, for MinGW is Windows XP
#if defined(_WIN32_WINNT) && defined(IO_BUILD)

#	if _WIN32_WINNT < 0x0600
#		undef  _WIN32_WINNT
#		define WINVER 0x0600
#		define _WIN32_WINNT 0x0600
#	endif // _WIN32_WINNT

#	define WIN32_LEAN_AND_MEAN

#elif !defined(_WIN32_WINNT)

#	define WINVER _WIN32_WINNT_VISTA
#	define _WIN32_WINNT _WIN32_WINNT_VISTA

#elif ( _WIN32_WINNT < _WIN32_WINNT_VISTA )

#	error "Windows Vista is minimal supported Windows version, \
			you mind fogot to define _WIN32_WINNT when on MinGW  \
			or check your winver.h in visual studio project "

#endif // defined

#ifdef WINVER
#	undef WINVER
#	define WINVER _WIN32_WINNT_VISTA
#endif // WINVER

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
