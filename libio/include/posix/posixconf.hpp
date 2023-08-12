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

#define IO_PUBLIC_SYMBOL

#define IO_MALLOC_ATTR __attribute__ ((malloc))

#include  "memory_traits.hpp"

typedef int os_descriptor_t;

#endif // __IO_POSIXCONF_HPP_INCLUDED__
