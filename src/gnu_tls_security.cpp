/*
 *
 * Copyright (c) 2016-2021
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#include "stdafx.hpp"
#include "gnu_tls_security.hpp"

namespace io {

namespace net {

namespace detail {

// synch_transport
synch_transport::synch_transport(io::s_read_write_channel&& raw) noexcept:
    transport(),
    raw_( std::forward<io::s_read_write_channel>(raw) )
{}

ssize_t synch_transport::pull(std::error_code& ec,void* dst,std::size_t len) noexcept
{
    size_t ret = raw_->read(ec, static_cast<uint8_t*>(dst), len);
    return ec ? -1 : ret;
}

ssize_t synch_transport::push(std::error_code& ec, const void* src,std::size_t len) noexcept
{
    size_t ret = raw_->write(ec, static_cast<const uint8_t*>(src), len);
    return ec ? -1 : ret;
}

// asynch_transport

asynch_transport::asynch_transport(io::s_asynch_channel&& raw) noexcept:
    transport(),
	raw_( std::forward<io::s_asynch_channel>(raw) )
{}

ssize_t asynch_transport::pull(std::error_code& ec, void* dst,std::size_t len) noexcept
{
	raw_->recaive(ec, len, 0 );
	// TODO: wait for completion and notify
	return -1;
}

ssize_t asynch_transport::push(std::error_code& ec, const void* src,std::size_t len) noexcept
{
	raw_->send(ec, io::byte_buffer::wrap(ec, static_cast<const uint8_t*>(src), len), 0 );
	// TODO: wait for completion and notify
	return -1;
}

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
        ret += s->connection()->push(s->ec_, buff->iov_base, buff->iov_len);
        if(s->ec_)
            break;
    }
    return ret;
}

ssize_t session_pull(::gnutls_transport_ptr_t tr, void * data, std::size_t max_size) noexcept
{
    session* s = static_cast<session*>( tr );
    return s->connection()->pull(s->ec_, data, max_size);
}

session::session(gnutls_session_t peer, detail::s_transport&& connection) noexcept:
    peer_( peer ),
    connection_( std::forward<detail::s_transport>(connection) ),
    ec_()
{}

session::~session() noexcept
{
    ::gnutls_bye(peer_, GNUTLS_SHUT_RDWR);
    ::gnutls_deinit( peer_ );
}

int session::client_handshake(::gnutls_session_t const peer) noexcept
{
    int ret = GNUTLS_E_SUCCESS;
    do {
        ret = ::gnutls_handshake( peer );
    }
    while( ret != GNUTLS_E_SUCCESS && 0 == ::gnutls_error_is_fatal(ret) );
    return ret;
}

s_session session::client_blocking_session(std::error_code &ec, ::gnutls_certificate_credentials_t crd,s_read_write_channel&& raw) noexcept
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
    if(ec)
        return s_session();
    detail::synch_transport *connection = new (std::nothrow) detail::synch_transport( std::forward<s_read_write_channel>(raw) );
    if(nullptr ==  connection) {
        ec = std::make_error_code(std::errc::not_enough_memory);
        return s_session();
    }
    session *ret = new (std::nothrow) session(peer, detail::s_transport( connection ) );
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


// tls_async_channel


} // namespace detail

// security_context
s_security_context security_context::create(std::error_code& ec,const io::s_io_context& ioc) noexcept
{
    if( GNUTLS_E_SUCCESS == ::gnutls_global_init() ) {
//#ifndef NO_DEBUG
//			gnutls_global_set_log_level(2);
//				::gnutls_global_set_log_function([] (int e, const char *msg) {
//					std::fprintf(stderr, "%i %s", e, msg);
//				});
//#endif // NO_DEBUG
        detail::credentials xcred = detail::credentials::system_trust_creds(ec);
        if(!ec) {
            s_security_context ret = new (std::nothrow) security_context( ioc, std::move(xcred) );
            if(nullptr == ret)
                ec = std::make_error_code(std::errc::not_enough_memory);
            else
                return s_security_context(ret);
        }
    }
    else {
        ec = std::make_error_code(std::errc::protocol_error);
    }
	return s_security_context();
}

security_context::security_context(const io::s_io_context& ioc, detail::credentials&& creds) noexcept:
    io::object(),
    ioc_(ioc),
    creds_( std::forward<detail::credentials>(creds) )
{}

security_context::~security_context() noexcept
{
    ::gnutls_global_deinit();
}

s_read_write_channel security_context::client_blocking_connect(std::error_code& ec,socket&& socket) const noexcept
{
    s_read_write_channel raw = ioc_->client_blocking_connect(ec, std::forward<io::net::socket>(socket) );
    if( !ec ) {
        detail::s_session clnts = detail::session::client_blocking_session(ec, creds_.get(),  std::move(raw) );
        if( !ec ) {
			detail::tls_channel *ret = new (std::nothrow) detail::tls_channel( std::move(clnts) );
            if( nullptr != ret )
				return s_read_write_channel( ret );
			else
				ec = std::make_error_code(std::errc::not_enough_memory );
        }
    }
    return s_read_write_channel();
}

s_read_write_channel security_context::client_blocking_connect(std::error_code& ec,const char* host, uint16_t port) const noexcept
{
	const io::net::socket_factory *sf = io::net::socket_factory::instance(ec);
	if(!ec) {
		return client_blocking_connect(ec, sf->client_tcp_socket(ec,host,port) );
	}
	return s_read_write_channel();
}

s_asynch_channel security_context::client_asynch_connect(std::error_code& ec, const s_asynch_io_context& aioc, net::socket&& socket,const s_asynch_completion_routine& routine) const noexcept
{

}


} // namespace net

} // namespace io
