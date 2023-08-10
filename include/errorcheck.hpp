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
#ifndef __IO_ERRORCHECK_HPP_INCLUDED__
#define __IO_ERRORCHECK_HPP_INCLUDED__

#include "config.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <system_error>

// choose for panic and implementation
#ifdef __IO_WINDOWS_BACKEND__
#	include "win/errorcheck.hpp"
#elif defined(__IO_POSIX_BACKEND__)
#   include "posix/errorcheck.hpp"
#endif // __IO_POSIX_BACKEND__

#include <functional>

namespace io {

/// Checks error code variable, if there is an error calls a fall back
/// \param fallback a fail back to be called in case of error
/// \param ec reference to error code variable
inline void check_error_code(const std::error_code& ec,const std::function<void(const std::error_code&)> &fallback) noexcept
{
    if( ec )
        fallback( ec );
}

/// Checks error code variable, if there is an error
/// prints error message into standard error stream
/// and do abnormal flow termination by
///     calling std::exit when exceptions off
/// 	throws std::system_error when exceptions is on
/// \param ec reference to error code variable
inline void check_error_code(const std::error_code &ec)
{
    if(ec)
#ifdef IO_NO_EXCEPTIONS
        check_error_code(
			ec,
			[] (const std::error_code& ec) {
				detail::panic(ec.value(), ec.message().data() );
			}
		);
#else
        check_error_code(
			ec,
			[] (const std::error_code& ec) {
				throw std::system_error( ec, ec.message().data() );
			}
		);
#endif // IO_NO_EXCEPTIONS
}


} //  namespace io


#endif // __IO_ERRORCHECK_HPP_INCLUDED__
