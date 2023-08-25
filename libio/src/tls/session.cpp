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
#include "session.hpp"

namespace io {

namespace net {

namespace tls {

// friend transport functions
int session_timeout(::gnutls_transport_ptr_t, unsigned int ms) noexcept
{
	 return static_cast<int>( ms);
}

ssize_t session_push(::gnutls_transport_ptr_t tr, const ::giovec_t * iov, int iovcnt) noexcept
{
	 session* s = static_cast<session*>( tr );
	 ssize_t ret = 0;
	 ssize_t written = 0;
	 for(std::size_t i = 0; i < static_cast<std::size_t>(iovcnt); i++) {
		written = s->push(iov[i].iov_base, iov[i].iov_len);
		if(written < 0) {
				ret = -1;
				break;
		}
		else {
				ret += written;
		}
	 }
	 return ret;
}

ssize_t session_pull(::gnutls_transport_ptr_t tr, void* buff, std::size_t buff_size) noexcept
{
	 session* s = static_cast<session*>( tr );
	 return s->pull( buff, buff_size);
}

// session
session::session(gnutls_session_t peer, s_transport&& connection) noexcept:
	 peer_( peer ),
	 connection_( std::forward<s_transport>(connection) )
{}

session::~session() noexcept
{
	int res;
	do {
		res = ::gnutls_bye(peer_, GNUTLS_SHUT_RDWR);
	}
	while( GNUTLS_E_INTERRUPTED == res || GNUTLS_E_AGAIN == res);
	::gnutls_deinit( peer_ );
}

ssize_t session::push(const void *data, std::size_t size) noexcept
{
	return connection_->push(data, size);
}

ssize_t session::pull(void *data, std::size_t size) noexcept
{
	return connection_->pull(data, size);
}

int session::client_handshake(::gnutls_session_t const peer) noexcept
{
	int ret = ::gnutls_handshake(peer);
	while ( ! ::gnutls_error_is_fatal(ret) ) {
		if ( 1 != ::gnutls_record_get_direction(peer) )
			break;
		else
			ret = ::gnutls_handshake(peer);
	}
	return ret;
}

s_session session::client_blocking_session(std::error_code &ec, ::gnutls_certificate_credentials_t crd,s_read_write_channel&& raw) noexcept
{
	s_session ret;
	::gnutls_session_t peer = nullptr;
	int err = ::gnutls_init(&peer, GNUTLS_CLIENT);
	if(GNUTLS_E_SUCCESS != err) {
		ec = make_error_code( err );
	} else {
		/* Use default priorities */
		//::gnutls_session_enable_compatibility_mode(peer);
		err = ::gnutls_set_default_priority(peer);
		if(GNUTLS_E_SUCCESS != err) {
			ec = make_error_code( err );
		} else {
			err = ::gnutls_credentials_set(peer, GNUTLS_CRD_CERTIFICATE, crd);
			if(GNUTLS_E_SUCCESS != err) {
				ec = make_error_code( err );
			} else {
				synch_transport *connection = new (std::nothrow) synch_transport( std::forward<s_read_write_channel>(raw) );
				if(nullptr ==  connection) {
					ec = std::make_error_code(std::errc::not_enough_memory);
				} else {
					session *px = new (std::nothrow) session(peer, s_transport( connection ) );
					if( nullptr == px ) {
						ec = std::make_error_code(std::errc::not_enough_memory);
					} else {
						ret.reset(px, true);
						::gnutls_transport_set_ptr(peer, static_cast<::gnutls_transport_ptr_t>( px ) );
						::gnutls_transport_set_pull_function(peer, session_pull);
						::gnutls_transport_set_vec_push_function(peer, session_push );
						//::gnutls_transport_set_pull_timeout_function(peer, ::gnutls_system_recv_timeout);
						err = client_handshake( peer );
						if(GNUTLS_E_SUCCESS != err)
							ec = make_error_code(err);
					}
				}
			}
		}
	}
	return ret;
}


std::size_t session::read(std::error_code& ec, uint8_t * const data, std::size_t max_size) const noexcept
{
	ssize_t ret;
	do {
		ret = ::gnutls_record_recv(peer_, static_cast<void*>(data), max_size);
	}
	while( (ret == GNUTLS_E_AGAIN || ret == GNUTLS_E_INTERRUPTED) && !::gnutls_record_get_direction(peer_));
	if( ret < 0 ) {
		ec = connection_->last_error();
		if(!ec && ::gnutls_error_is_fatal( static_cast<int>(ret) ) )
				ec = make_error_code( static_cast<int>(ret) );
		return 0;
	}
	return static_cast<std::size_t>( ret );
}

std::size_t session::write(std::error_code& ec, const uint8_t *data, std::size_t data_size) const noexcept
{
	ssize_t ret = ::gnutls_record_send(peer_, static_cast<const void*>(data), data_size);
	while( GNUTLS_E_INTERRUPTED == ret || GNUTLS_E_AGAIN  == ret) {
		ret = ::gnutls_record_send(peer_, nullptr, 0);
	}
	if( ret < 0 ) {
		ec = connection_->last_error();
		if(!ec && ::gnutls_error_is_fatal( static_cast<int>(ret) ) )
				ec = make_error_code( static_cast<int>(ret) );
		return 0;
	}
	return static_cast<std::size_t>( ret );
}


} // namespace tls

} //namespace net

} // namespace io
