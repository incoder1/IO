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

namespace secure {

// credentials

credentials credentials::system_trust_creds(std::error_code& ec) noexcept
{
	credentials ret;
	if( GNUTLS_E_SUCCESS != ::gnutls_certificate_allocate_credentials( &ret.creds_ ) )
		ec = std::make_error_code( std::errc::protocol_error );
	else if( gnutls_certificate_set_x509_system_trust( ret.creds_ ) < 0 )
		ec = std::make_error_code( std::errc::protocol_error );
	return ret;
}

// session
session::session(session&& other) noexcept:
	peer_( other.peer_ ),
	socket_( std::move( other.socket_ ) )
{
	other.peer_ = nullptr;
}

session::~session() noexcept
{
	if(nullptr != peer_)
		::gnutls_deinit( peer_ );
}

ssize_t session::push(::gnutls_transport_ptr_t t, const void * data, std::size_t size)
{
	read_write_channel *tr = reinterpret_cast<read_write_channel*>(t);
	std::error_code ec;
	std::size_t ret = tr->write( ec, static_cast<const uint8_t*>(data), size);
	return ec ?  -1 : ret;
}

ssize_t session::pull(::gnutls_transport_ptr_t t, void * data, std::size_t max_size)
{
	read_write_channel *tr = reinterpret_cast<read_write_channel*>(t);
	std::error_code ec;
	std::size_t ret = tr->read( ec, static_cast<uint8_t*>(data), max_size);
	return ec ?  -1 : ret;
}

int session::client_handshake(::gnutls_session_t const peer) noexcept
{
	::gnutls_handshake_set_timeout(peer, GNUTLS_DEFAULT_HANDSHAKE_TIMEOUT);
	::gnutls_handshake_set_timeout(peer, GNUTLS_INDEFINITE_TIMEOUT);
	int ret;
	do {
		ret = ::gnutls_handshake(peer);
	}
	while(0 != ret && (::gnutls_error_is_fatal(ret) > 0) );
	return ret;
}

session session::client_session(std::error_code &ec, ::gnutls_certificate_credentials_t crd,s_read_write_channel&& socket) noexcept
{
	session ret( std::forward<s_read_write_channel>(socket) );
	int err = ::gnutls_init( &ret.peer_, GNUTLS_CLIENT);
	if(err > 0)
		ec = std::make_error_code( std::errc::connection_refused );
	err = ::gnutls_set_default_priority(ret.peer_);
	if(err > 0 )
		ec = std::make_error_code( std::errc::connection_refused );

	err = ::gnutls_credentials_set(ret.peer_, GNUTLS_CRD_CERTIFICATE, crd);
	if(err > 0 )
		ec = std::make_error_code( std::errc::connection_refused );

	::gnutls_transport_ptr_t transport = reinterpret_cast<::gnutls_transport_ptr_t>( ret.socket_.get() );

	::gnutls_transport_set_ptr( ret.peer_, transport );
	::gnutls_transport_set_push_function( ret.peer_, &session::push );
	::gnutls_transport_set_pull_function( ret.peer_, &session::pull );

	err = client_handshake( ret.peer_ );
	if(GNUTLS_E_SUCCESS != err)
		ec = std::make_error_code( std::errc::connection_refused );
	return ret;
}


session::session(s_read_write_channel&& socket) noexcept:
	peer_(nullptr),
	socket_( std::forward<s_read_write_channel>( socket ) )
{
}

std::size_t session::read(std::error_code& ec, uint8_t * const data, std::size_t max_size) const noexcept
{
	ssize_t ret = ::gnutls_record_recv(peer_, static_cast<void*>(data), max_size);
	if( ret < 0 ) {
		ec  = std::make_error_code(std::errc::broken_pipe);
		return 0;
	}
	return static_cast<std::size_t>( ret );
}

std::size_t session::write(std::error_code& ec, const uint8_t *data, std::size_t data_size) const noexcept
{
	ssize_t ret = ::gnutls_record_send(peer_, static_cast<const void*>(data), data_size);
	if( ret < 0 ) {
		ec  = std::make_error_code(std::errc::broken_pipe);
		return 0;
	}
	return static_cast<std::size_t>( ret );
}


//tls_channel

tls_channel::tls_channel(session&& s) noexcept:
	read_write_channel(),
	session_( std::forward<session>(s) )
{}

tls_channel::~tls_channel() noexcept
{}

std::size_t tls_channel::read(std::error_code& ec,uint8_t* const buff, std::size_t bytes) const noexcept
{
	return session_.read(ec, buff, bytes);
}

std::size_t tls_channel::write(std::error_code& ec, const uint8_t* buff,std::size_t size) const noexcept
{
	return session_.write(ec, buff, size);
}

// service
std::atomic<service*> service::_instance(nullptr);
critical_section service::_mtx;

void service::destroy_gnu_tls_atexit() noexcept
{
	service *srv = _instance.load(std::memory_order_relaxed);
	if(nullptr != srv) {
		::gnutls_global_deinit();
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

s_read_write_channel service::new_client_connection(std::error_code& ec, s_read_write_channel&& socket) const noexcept
{

	session s = session::client_session(ec, creds_.get(),  std::forward<s_read_write_channel>(socket) );
	if( ec )
		return s_read_write_channel();
	tls_channel* ch = nobadalloc< tls_channel >::construct(ec, std::move(s) );
	return ec ? s_read_write_channel() : s_read_write_channel( ch );
}


} // namespace secure

} // namespace io
