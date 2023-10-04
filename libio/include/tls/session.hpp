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
#ifndef __IO_TLS_SESSION_HPP_INCLUDED__
#define __IO_TLS_SESSION_HPP_INCLUDED__

#include "config.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include "credentials.hpp"
#include "transport.hpp"
#include "tls_error.hpp"

namespace io {

namespace net {

namespace tls {

class session;
DECLARE_IPTR(session);

class session final:public object {

	session(const session&) = delete;
	session& operator=(const session&) = delete;

public:

	static s_session client_blocking_session(std::error_code &ec, ::gnutls_certificate_credentials_t crd, s_read_write_channel&& raw) noexcept;


	~session() noexcept;

	inline void swap(session& other) noexcept
	{
		std::swap(peer_, other.peer_);
	}

	std::size_t read(std::error_code& ec, uint8_t * const data, std::size_t max_size) const noexcept;

	std::size_t write(std::error_code& ec, const uint8_t *data, std::size_t data_size) const noexcept;

private:

	session(::gnutls_session_t peer, s_transport&& connection) noexcept;

	ssize_t push(const void *data, std::size_t size) noexcept;

	ssize_t pull(void *data, std::size_t size) noexcept;

	static int client_handshake(::gnutls_session_t peer) noexcept;

	friend int session_timeout(::gnutls_transport_ptr_t, unsigned int ms) noexcept;

	friend ssize_t session_push(::gnutls_transport_ptr_t tr, const ::giovec_t * iov, int iovcnt) noexcept;

	friend ssize_t session_pull(::gnutls_transport_ptr_t tr, void* buff, std::size_t buff_size) noexcept;

private:
	::gnutls_session_t peer_;
	s_transport connection_;
};


} // namespace tls

} //namespace net

} // namespace io

#endif // __IO_TLS_SESSION_HPP_INCLUDED__
