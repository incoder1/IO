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

#ifdef IO_HAS_BOOST

#ifdef IO_NO_EXCEPTIONS

#include <exception>

// to be used when we have boost toolchain
namespace boost
{
	inline void throw_exception( std::system_error const & e )
	{
		detail::panic( e.code() , e.what() );
	}
}
#endif // IO_NO_EXCEPTIONS

#endif // IO_HAS_BOOST

namespace io {

/// Checks error code variable, if there is an error
/// prints error message into standard error stream
/// and do abonormal flow termination by:
/// 	calling std::terminate when exceptions is off
/// 	(can be instercepted with terminate handler).
/// 	throws std::system_error when exceptions is on
/// \param ec reference to error code variable
inline void check_error_code(std::error_code const  &ec)
{
	if(ec)
#ifdef IO_NO_EXCEPTIONS
        detail::panic(ec.value(), ec.message().c_str() );
#else
		throw std::system_error( ec, ec.message().c_str() );
#endif // IO_NO_EXCEPTIONS
}


} //  namespace io


#endif // __IO_ERRORCHECK_HPP_INCLUDED__
