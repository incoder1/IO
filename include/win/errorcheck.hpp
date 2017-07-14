#ifndef __IO_WIN_ERROR_CHECK_HPP_INCLUDED__
#define __IO_WIN_ERROR_CHECK_HPP_INCLUDED__

#include "winconf.hpp"

#if defined(__GNUC__) && defined(IO_SHARED_LIB)
#	ifdef IO_BUILD
#		define IO_PANIC_ATTR __attribute__ ((__noreturn__,dllexport))
#	else
#		define IO_PANIC_ATTR __attribute__ ((__noreturn__,dllimport))
#	endif // IO_BUILD
#elif defined(__GNUC__)
#	define IO_PANIC_ATTR __attribute__ ((__noreturn__))
#endif // __GNUC__

#if defined(_MSC_VER) && defined(IO_SHARED_LIB)
#	ifdef IO_BUILD
#		define IO_PANIC_ATTR __declspec(noreturn,dllexport)
#	else
#		define IO_PANIC_ATTR __declspec(noreturn,dllimport)
#	endif // IO_BUILD
#elif defined(_MSC_VER)
#	define IO_PANIC_ATTR __declspec(noreturn)
#endif // defined

#ifndef IO_PANIC_ATTR
#	define IO_PANIC_ATTR
#endif // IO_PANIC_ATTR

namespace io {

namespace detail {

extern "C" void IO_PANIC_ATTR panic(int errcode, const char* message);

} // namespace detail

extern "C" void IO_PANIC_ATTR exit_with_current_error();
extern "C" void IO_PANIC_ATTR exit_with_error_message(int exitcode, const char* message);

} // namespace io

#endif // __IO_WIN_ERROR_CHECK_HPP_INCLUDED__
