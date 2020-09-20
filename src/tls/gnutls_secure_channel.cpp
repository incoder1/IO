/*
 *
 * Copyright (c) 2017
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#include "stdafx.hpp"
#include "tls/gnutls_secure_channel.hpp"
#include "threading.hpp"

namespace io {

namespace net {

namespace secure {

// credentials

credentials credentials::system_trust_creds(std::error_code& ec) noexcept
{
	credentials ret;
	if( GNUTLS_E_SUCCESS != ::gnutls_certificate_allocate_credentials( &ret.creds_ ) )
		ec = std::make_error_code( std::errc::protocol_error );
	else if( GNUTLS_E_UNIMPLEMENTED_FEATURE == ::gnutls_certificate_set_x509_system_trust( ret.creds_ ) )
		ec = std::make_error_code( std::errc::protocol_error );
	return ret;
}


// session

// friend transport functions
int session_timeout(::gnutls_transport_ptr_t tr, unsigned int ms) noexcept
{
	return static_cast<int>( ms);
}

int get_session_error_no(::gnutls_transport_ptr_t tr) noexcept
{
	session* s = static_cast<session*>( tr );
	return s->ec_.value();
}

ssize_t session_push(::gnutls_transport_ptr_t tr,const ::giovec_t* buffers, int bcount) noexcept
{
	session* s = static_cast<session*>( tr );
	ssize_t ret = 0;
	for(unsigned i=0; i < static_cast<unsigned>(bcount); i++) {
		const giovec_t* buff = buffers + i;
		ret += s->raw_write(static_cast<const uint8_t*>(buff->iov_base), buff->iov_len);
		if(s->ec_)
			break;
	}
	return ret;
}

ssize_t session_pull(::gnutls_transport_ptr_t tr, void * data, std::size_t max_size) noexcept
{
	session* s = static_cast<session*>( tr );
	return s->raw_read(static_cast<uint8_t*>(data), max_size);
}

session::session(gnutls_session_t peer,s_socket&& socket, s_read_write_channel&& connection) noexcept:
	peer_( peer ),
	socket_( std::forward<s_socket>(socket) ),
	connection_( std::forward<s_read_write_channel>(connection) ),
	ec_()
{}

session::~session() noexcept
{
	::gnutls_bye(peer_, GNUTLS_SHUT_RDWR);
	::gnutls_deinit( peer_ );
}

int session::client_handshake(::gnutls_session_t const peer) noexcept
{
//	::gnutls_handshake_set_timeout(peer, GNUTLS_DEFAULT_HANDSHAKE_TIMEOUT);
	int ret = GNUTLS_E_SUCCESS;
	do {
		ret = ::gnutls_handshake( peer );
	} while( ret != GNUTLS_E_SUCCESS && 0 == gnutls_error_is_fatal(ret) );
	return ret;
}

s_session session::client_session(std::error_code &ec, ::gnutls_certificate_credentials_t crd,s_socket&& socket) noexcept
{
	::gnutls_session_t peer = nullptr;
	int err = ::gnutls_init(&peer, GNUTLS_CLIENT);
	if(GNUTLS_E_SUCCESS != err) {
		ec = std::make_error_code( std::errc::connection_refused );
		return s_session();
	}
	/* Use default priorities */
	::gnutls_session_enable_compatibility_mode(peer);
	err = ::gnutls_set_default_priority(peer);
	if(GNUTLS_E_SUCCESS != err) {
		ec = std::make_error_code( std::errc::connection_refused );
		return s_session();
	}
	err = ::gnutls_credentials_set(peer, GNUTLS_CRD_CERTIFICATE, crd);
	if(GNUTLS_E_SUCCESS != err) {
		ec = std::make_error_code( std::errc::connection_refused );
		return s_session();
	}
	s_read_write_channel connection = socket->connect(ec);
	if(ec)
		return s_session();
	session *ret = new (std::nothrow) session(peer, std::move(socket), std::move(connection) );
	if( nullptr == ret ) {
		ec = std::make_error_code(std::errc::not_enough_memory);
		return s_session();
	}
	::gnutls_transport_set_ptr(peer, static_cast<::gnutls_transport_ptr_t>( ret ) );
	::gnutls_transport_set_vec_push_function(peer, session_push );
	::gnutls_transport_set_pull_function(peer, session_pull );

	::gnutls_transport_set_pull_timeout_function(peer, ::gnutls_system_recv_timeout);
	::gnutls_transport_set_errno_function(peer,get_session_error_no);

	err = client_handshake( peer );
	switch(err) {
	case GNUTLS_E_SUCCESS:
		break;
	case GNUTLS_E_CERTIFICATE_VERIFICATION_ERROR:
		ec = std::make_error_code( std::errc::operation_canceled );
		return s_session();
	default:
		ec = ret->ec_;
		return s_session();
	}
	return s_session(ret);
}

ssize_t session::raw_read(uint8_t* dst,std::size_t len) noexcept
{
	size_t ret = connection_->read(ec_, dst, len);
	return ec_ ? -1 : ret;
}

ssize_t session::raw_write(const uint8_t* src,std::size_t len) noexcept
{
	size_t ret = connection_->write(ec_, src, len);
	return ec_ ? -1 : ret;
}


std::size_t session::read(std::error_code& ec, uint8_t * const data, std::size_t max_size) const noexcept
{
	ssize_t ret = ::gnutls_record_recv(peer_, static_cast<void*>(data), max_size);
	if( ret < 0 && 0 != ::gnutls_error_is_fatal(ret) ) {
        ec = ec_;
		return 0;
	}
	return static_cast<std::size_t>( ret );
}

std::size_t session::write(std::error_code& ec, const uint8_t *data, std::size_t data_size) const noexcept
{
	ssize_t ret = ::gnutls_record_send(peer_, static_cast<const void*>(data), data_size);
	if( ret < 0 && 0 != ::gnutls_error_is_fatal(ret) ) {
		ec = ec_;
		return 0;
	}
	return static_cast<std::size_t>( ret );
}


//tls_channel

tls_channel::tls_channel(s_session&& s) noexcept:
	read_write_channel(),
	session_( std::forward<s_session>(s) )
{}

tls_channel::~tls_channel() noexcept
{}

std::size_t tls_channel::read(std::error_code& ec,uint8_t* const buff, std::size_t bytes) const noexcept
{
	return session_->read(ec, buff, bytes);
}

std::size_t tls_channel::write(std::error_code& ec, const uint8_t* buff,std::size_t size) const noexcept
{
	return session_->write(ec, buff, size);
}

// service
std::atomic<service*> service::_instance(nullptr);
critical_section service::_mtx;

void service::destroy_gnu_tls_atexit() noexcept
{
	::gnutls_global_deinit();
	service *srv = _instance.load(std::memory_order_relaxed);
	if(nullptr != srv) {
		delete srv;
	}
}

const service* service::instance(std::error_code& ec) noexcept
{
	service *ret = _instance.load(std::memory_order_consume);
	if(nullptr == ret) {
		lock_guard lock(_mtx);
		ret = _instance.load(std::memory_order_consume);
		if(nullptr == ret) {
			if( GNUTLS_E_SUCCESS == ::gnutls_global_init() ) {

//				gnutls_global_set_log_level(2);
//				::gnutls_global_set_log_function([] (int e, const char *msg) {
//					std::fprintf(stderr, "%i %s", e, msg);
//				});

				credentials xcred = credentials::system_trust_creds(ec);
				if(!ec) {
					std::atexit( &service::destroy_gnu_tls_atexit );
					ret = new (std::nothrow) service( std::move(xcred) );
					if(nullptr == ret)
						ec = std::make_error_code(std::errc::not_enough_memory);
				}
			}
			else
				ec = std::make_error_code(std::errc::io_error);
			_instance.store(ret, std::memory_order_release);
		}
	}
	return ret;
}

service::~service() noexcept
{}

s_read_write_channel service::new_client_blocking_connection(std::error_code& ec,const char* host,const uint16_t port) const noexcept
{
	const socket_factory* sf = socket_factory::instance(ec);
	if(ec)
		return s_read_write_channel();
	s_socket socket = sf->client_tcp_socket(ec, host, port);
	if(ec)
		return s_read_write_channel();

	s_session s = session::client_session(ec, creds_.get(),  std::forward<s_socket>(socket) );
	if( ec )
		return s_read_write_channel();
	tls_channel* ch = nobadalloc< tls_channel >::construct(ec, std::move(s) );
	return ec ? s_read_write_channel() : s_read_write_channel( ch );
}


} // namespace secure

} // namespace net

} // namespace io
