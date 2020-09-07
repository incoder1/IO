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
#ifndef __IO_WSAERROR_HPP_INCLUDED__
#define __IO_WSAERROR_HPP_INCLUDED__

#include "config.hpp"


#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <system_error>

namespace io {

namespace net {

enum class err_code: int
{
  /// Permission denied.
  access_denied = WSAEACCES,

  /// Address family not supported by protocol.
  address_family_not_supported = WSAEAFNOSUPPORT,

  /// Address already in use.
  address_in_use = WSAEADDRINUSE,

  /// Transport endpoint is already connected.
  already_connected = WSAEISCONN,

  /// Operation already in progress.
  already_started = WSAEALREADY,

  /// Broken pipe.
  broken_pipe = ERROR_BROKEN_PIPE,

  /// A connection has been aborted.
  connection_aborted = WSAECONNABORTED,

  /// Connection refused.
  connection_refused = WSAECONNREFUSED,

  /// Connection reset by peer.
  connection_reset = WSAECONNRESET,

  /// Bad file descriptor.
  bad_descriptor = WSAEBADF,

  /// Bad address.
  fault = WSAEFAULT,

  /// No route to host.
  host_unreachable = WSAEHOSTUNREACH,

  /// Operation now in progress.
  in_progress = WSAEINPROGRESS,

  /// Interrupted system call.
  interrupted = WSAEINTR,

  /// Invalid argument.
  invalid_argument = WSAEINVAL,

  /// Message too long.
  message_size = WSAEMSGSIZE,

  /// The name was too long.
  name_too_long = WSAENAMETOOLONG,

  /// Network is down.
  network_down = WSAENETDOWN,

  /// Network dropped connection on reset.
  network_reset = WSAENETRESET,

  /// Network is unreachable.
  network_unreachable = WSAENETUNREACH,

  /// Too many open files.
  no_descriptors = WSAEMFILE,

  /// No buffer space available.
  no_buffer_space = WSAENOBUFS,

  /// Cannot allocate memory.
  no_memory = ERROR_OUTOFMEMORY,

  /// Operation not permitted.
  no_permission = ERROR_ACCESS_DENIED,

  /// Protocol not available.
  no_protocol_option = WSAENOPROTOOPT,

  /// No such device.
  no_such_device = ERROR_BAD_UNIT,

  /// Transport endpoint is not connected.
  not_connected = WSAENOTCONN,

  /// Socket operation on non-socket.
  not_socket = WSAENOTSOCK,

  /// Operation cancelled.
  operation_aborted = ERROR_OPERATION_ABORTED,

  /// Operation not supported.
  operation_not_supported = WSAEOPNOTSUPP,

  /// Cannot send after transport endpoint shutdown.
  shut_down = WSAESHUTDOWN,

  /// Connection timed out.
  timed_out = WSAETIMEDOUT,

  /// Resource temporarily unavailable.
  try_again = ERROR_RETRY,

  /// The socket is marked non-blocking and the requested operation would block.
  would_block = WSAEWOULDBLOCK
};

class IO_PUBLIC_SYMBOL error_category final: public std::error_category
{
public:
	constexpr error_category() noexcept:
		std::error_category()
	{}

	virtual ~error_category() = default;

	virtual const char* name() const noexcept override;

	virtual std::error_condition default_error_condition (int err) const noexcept override;

	virtual bool equivalent (const std::error_code& code, int condition) const noexcept override;

	virtual std::string message(int err_code) const override;
};

#ifndef  _MSC_VER
	_GLIBCXX_CONST const error_category& network_category() noexcept;
#else
	const error_category& network_category() noexcept;
#endif

std::error_code make_wsa_last_error_code() noexcept;

} // namespace net

} // namespace io

// extending standard name space for error code
namespace std {

template<>
struct is_error_code_enum<io::net::err_code>:
	public true_type
{};

} // namespace std


#endif // __IO_WSAERROR_HPP_INCLUDED__
