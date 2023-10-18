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
#include "stdafx.hpp"
#include "io/core/posix/errorcheck.hpp"

#ifndef IO_NO_EXCEPTIONS
#	include <ios>
#endif // IO_NO_EXCEPTIONS

namespace io {

extern "C" {

void IO_PANIC_ATTR exit_with_error_message(int exitcode, const char* message)
{
	std::fprintf(stderr, "\033[01;31m %i %s \033[0m\n", exitcode, message);
	std::exit(exitcode);
}

} // extern "C"

namespace detail {

void IO_PUBLIC_SYMBOL ios_check_error_code(const char* msg, std::error_code const &ec )
{
	if(ec) {
#ifdef IO_NO_EXCEPTIONS
		std::fprintf(stderr, "\033[01;31m %i %s %s \033[0m\n", ec.value(), msg, ec.message().data() );
		std::exit( ec.value() );
#else
		throw std::ios_base::failure( std::string(msg) + ec.message() );
#endif
	}
}

} // namespace detail

} // namespace io
