#ifndef __IO_POSIXCONF_HPP_INCLUDED__
#define __IO_POSIXCONF_HPP_INCLUDED__

#include <unistd.h>

// shared library
#if defined(IO_SHARED_LIB) && defined(__GNUC__) || defined(__CLANG__)
#	define IO_PUBLIC_SYMBOL __attribute__ ((visibility ("default")))
// static library
#else
#	define IO_PUBLIC_SYMBOL
#endif // IO_SHARED_LIB

#include  "hmalloc.hpp"


#endif // __IO_POSIXCONF_HPP_INCLUDED__
