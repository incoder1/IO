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
#ifndef __IO_POSIX_ERRORCHECK_HPP_INCLUDED__
#define __IO_POSIX_ERRORCHECK_HPP_INCLUDED__

#include "posixconf.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <cerrno>
#include <cstdio>
#include <cstring>

#define IO_PANIC_ATTR __attribute__ ((__noreturn__))

namespace io {

namespace detail {

void IO_PANIC_ATTR panic(int errcode, const char* message);

void ios_check_error_code(const char* msg, std::error_code const &ec );

} // namespace detail

void IO_PANIC_ATTR exit_with_current_error();

void IO_PANIC_ATTR exit_with_error_message(int exitcode, const char* message);

} // namespace io


#endif // __IO_POSIX_ERRORCHECK_HPP_INCLUDED__
