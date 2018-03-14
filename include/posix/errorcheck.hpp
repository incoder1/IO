/*
 *
 * Copyright (c) 2017
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


inline void IO_PANIC_ATTR panic(int errcode, const char* message)
{
	static constexpr const char* __RED_FORMAT = "\033[01;31m %i %s \033[0m\n";
    std::fprintf(stderr, __RED_FORMAT, errcode, message);
    std::exit(errcode);
}

inline void ios_check_error_code(const char* msg, std::error_code const &ec )
	if(!ec)
		return;
#ifdef IO_NO_EXCEPTIONS
	static constexpr const char* __RED_FORMAT = "\033[01;31m%i %s%s\033[0m\n";
	std::fprintf(stderr, __RED_FORMAT, ec.value(), msg, ec.message().data() );
	std::exit( ec.value() );
#else
	throw std::ios_base::failure( msg, ec );
#endif
}


} // namespace detail

void inline IO_PANIC_ATTR exit_with_current_error()
{
    detail::panic(errno, std::strerror(errno) );
}

inline  void IO_PANIC_ATTR exit_with_error_message(int exitcode, const char* message)
{
    detail::panic(exitcode, message);
}

} // namespace io


#endif // __IO_POSIX_ERRORCHECK_HPP_INCLUDED__
