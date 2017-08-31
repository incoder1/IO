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

inline std::errc wsa_last_error_to_errc(int wsa_err) noexcept
{
	switch(wsa_err) {
	case WSAENOBUFS:
		return  std::errc::not_enough_memory ; //ENOMEM;
	case WSAEACCES:
		return  std::errc::permission_denied; // EACCES;
	case WSANOTINITIALISED:
		return  std::errc::operation_not_permitted;  // EPERM;
	case WSAEFAULT:
		return  std::errc::bad_address; // EFAULT;
	case WSAEINVAL:
		return std::errc::invalid_argument; // EINVAL;
	case WSAEWOULDBLOCK:
		return std::errc::resource_unavailable_try_again;
	case WSAEOPNOTSUPP:
		return  std::errc::function_not_supported;
	case WSAEMSGSIZE:
		return std::errc::no_space_on_device;
	case WSAENOTSOCK:
		return std::errc::no_such_device;
	case WSAVERNOTSUPPORTED:
	case WSAENOPROTOOPT:
		return std::errc::function_not_supported;
	case WSAECONNREFUSED:
		return std::errc::permission_denied;
	case WSAEAFNOSUPPORT:
		return std::errc::function_not_supported;
	case WSAEBADF:
		return std::errc::bad_file_descriptor;
	case WSAEINTR:
	case WSAENETRESET:
		return std::errc::interrupted;
	case WSAENOTCONN:
		return std::errc::resource_unavailable_try_again;
	case WSAEPROCLIM:
	case WSAEINPROGRESS:
		return std::errc::device_or_resource_busy;
	case WSAECONNABORTED:
		return std::errc::interrupted;
	case WSAECONNRESET:
		return std::errc::interrupted;
	case WSAESHUTDOWN:
		return std::errc::interrupted;
	case WSAETIMEDOUT:
		return std::errc::timed_out;
	case WSAHOST_NOT_FOUND:
        return std::errc::no_such_device_or_address;
	case WSAEHOSTUNREACH:
	case WSAENETDOWN:
	case WSASYSNOTREADY:
	default:
		return std::errc::io_error;
	}
}

inline std::errc wsa_last_error_to_errc() noexcept
{
	return wsa_last_error_to_errc( ::WSAGetLastError() );
}

} // namespace win

} // namespace net

} // namespace io


#endif // __IO_WSAERROR_HPP_INCLUDED__
