#ifndef __IO_WINCONG_HPP_INCLUDED__
#define __IO_WINCONG_HPP_INCLUDED__

#ifndef _WIN32_WINNT_VISTA
#	define _WIN32_WINNT_VISTA                  0x0600 // Windows Vista
#endif // _WIN32_WINNT_VISTA

// Check for the default value, for MinGW is Windows XP
#if defined(_WIN32_WINNT) && defined(IO_BUILD)

#	if _WIN32_WINNT < _WIN32_WINNT_VISTA
#		undef  _WIN32_WINNT
#		define WINVER _WIN32_WINNT_VISTA
#		define _WIN32_WINNT _WIN32_WINNT_VISTA
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

#include "hmalloc.hpp"

#endif // __IO_WINCONG_HPP_INCLUDED__
