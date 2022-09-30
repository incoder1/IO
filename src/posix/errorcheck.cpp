/*
 *
 * Copyright (c) 2016-2022
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#include "stdafx.hpp"
#include "posix/errorcheck.hpp"

#ifndef IO_NO_EXCEPTIONS
#   include <ios>
#endif // IO_NO_EXCEPTIONS

namespace io {

namespace detail {

void IO_PANIC_ATTR panic(int errcode, const char* message)
{
    std::fprintf(stderr, "\033[01;31m %i %s \033[0m\n", errcode, message);
    std::exit(errcode);
}

void ios_check_error_code(const char* msg, std::error_code const &ec )
{
	if(!ec)
		return;
#ifdef IO_NO_EXCEPTIONS
	std::fprintf(stderr, "\033[01;31m %i %s %s \033[0m\n", ec.value(), msg, ec.message().data() );
	std::exit( ec.value() );
#else
	throw std::ios_base::failure( msg + ec.message() );
#endif
}

} // namespace detail

void IO_PANIC_ATTR exit_with_current_error()
{
    detail::panic(errno, std::strerror(errno) );
}

void IO_PANIC_ATTR exit_with_error_message(int exitcode, const char* message)
{
    detail::panic(exitcode, message);
}

} // namespace io
