#ifndef __IO_ERRORCHECK_HPP_INCLUDED__
#define __IO_ERRORCHECK_HPP_INCLUDED__

#include "config.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <system_error>

#ifdef __IO_WINDOWS_BACKEND__
#	include "win/errorcheck.hpp"
#endif // __IO_WINDOWS_BACKEND__

#ifdef IO_HAS_BOOST

// #	include <boost/error>

#ifdef IO_NO_EXCEPTIONS

#include <exception>

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
