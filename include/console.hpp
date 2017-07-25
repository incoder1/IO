/*
 * Copyright (c) 2016-2017
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef __IO_CONSOLE_HPP_INCLUDED__
#define __IO_CONSOLE_HPP_INCLUDED__

#include "config.hpp"

#include "text.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#ifdef __IO_WINDOWS_BACKEND__
#	include "win/console.hpp"
#elif defined(__IO_POSIX_BACKEND__)
#	include "posix/console.hpp"
#endif // __IO_WINDOWS_BACKEND__

#endif // __IO_CONSOLE_HPP_INCLUDED__
