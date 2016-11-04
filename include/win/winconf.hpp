#ifndef __IO_WINCONG_HPP_INCLUDED__
#define __IO_WINCONG_HPP_INCLUDED__


#include <winsock2.h>
#include <windows.h>

// shared library
#if defined(IO_SHARED_LIB)
#	if defined(IO_BUILD_SHARED_LIB)
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
