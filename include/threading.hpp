/*
 * Copyright (c) 2016-2019
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef __IO_THREADING_HPP_INCLUDED__
#define __IO_THREADING_HPP_INCLUDED__

#include "config.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCEs

#if  defined(__IO_WINDOWS_BACKEND__)

#	include "win/criticalsection.hpp"
#	include "win/srw_lock.hpp"

#elif defined( __IO_POSIX_BACKEND__)

#	include "posix/criticalsection.hpp"
#	include "posix/rwlock.hpp"

#endif // __IO_WINDOWS_BACKEND__

#endif // __IO_THREADING_HPP_INCLUDED__
