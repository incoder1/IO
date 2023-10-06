/*
 *
 * Copyright (c) 2016-2023
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef __IO_NETWORK_HPP_INCLUDED__
#define __IO_NETWORK_HPP_INCLUDED__

#include <io/config/libio_config.hpp>

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#ifdef __IO_WINDOWS_BACKEND__
#	include "win/io_context.hpp"
#elif defined(__IO_POSIX_BACKEND__)
#	include "posix/io_context.hpp"
#endif // __IO_WINDOWS_BACKEND__

#endif // __IO_NETWORK_HPP_INCLUDED__
