#ifndef __IO_WSAERROR_HPP_INCLUDED__
#define __IO_WSAERROR_HPP_INCLUDED__

#include "config.hpp"


#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <system_error>

namespace io {

namespace net {

namespace win {

inline std::errc wsa_last_error_to_errc()
{
	switch ( ::WSAGetLastError() ) {
	case WSAENOBUFS:
		return  std::errc::not_enough_memory ; //ENOMEM;
	case WSAEACCES:
		return  std::errc::permission_denied; // EACCES;
	case WSANOTINITIALISED:
		return  std::errc::operation_not_permitted;  // EPERM;
	case WSAEHOSTUNREACH:
	case WSAENETDOWN:
		return  std::errc::io_error; // EIO;
	case WSAEFAULT:
		return  std::errc::bad_address; // EFAULT;
	case WSAEINTR:
		return std::errc::interrupted;
	case WSAEINVAL:
		return std::errc::invalid_argument; // EINVAL;
	case WSAEINPROGRESS:
		//errno = EINPROGRESS;
		return std::errc::device_or_resource_busy;
	case WSAEWOULDBLOCK:
		//errno = EAGAIN;
		return std::errc::resource_unavailable_try_again;
	case WSAEOPNOTSUPP:
		//errno = ENOTSUP;
		return  std::errc::function_not_supported;
	case WSAEMSGSIZE:
		//errno = EFBIG;
		return std::errc::file_too_large;
	case WSAENOTSOCK:
		//errno = ENOTSOCK;
		//break;
		return std::errc::no_such_device;
	case WSAENOPROTOOPT:
		// errno = ENOPROTOOPT;
		return std::errc::function_not_supported;
	case WSAECONNREFUSED:
		// errno = ECONNREFUSED;
		return std::errc::io_error;
	case WSAEAFNOSUPPORT:
		//errno = EAFNOSUPPORT;
		//break;
		return std::errc::invalid_argument;
	case WSAEBADF:
		//errno = EBADF;
		//break;
		return std::errc::bad_file_descriptor;
	case WSAENETRESET:
	case WSAENOTCONN:
	case WSAECONNABORTED:
	case WSAECONNRESET:
	case WSAESHUTDOWN:
	case WSAETIMEDOUT:
		return std::errc::broken_pipe;
	default:
		return std::errc::io_error;
	}
}

} // namespace win

} // namespace net

} // namespace io


#endif // __IO_WSAERROR_HPP_INCLUDED__
