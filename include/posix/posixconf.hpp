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
#ifndef __IO_POSIXCONF_HPP_INCLUDED__
#define __IO_POSIXCONF_HPP_INCLUDED__

#include <sys/types.h>
#include <unistd.h>

// shared library
#if defined(IO_SHARED_LIB) && defined(__GNUC__) || defined(__CLANG__)
#	define IO_PUBLIC_SYMBOL __attribute__ ((visibility ("default")))
// static library
#else
#	define IO_PUBLIC_SYMBOL
#endif // IO_SHARED_LIB

#define IO_MALLOC_ATTR __attribute__ ((dllexport,malloc))

#include  "memory_traits.hpp"

#endif // __IO_POSIXCONF_HPP_INCLUDED__
