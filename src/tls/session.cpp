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
#include "session.hpp"

namespace io {

namespace net {

namespace tls {

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

ssize_t session_push(::gnutls_transport_ptr_t tr, const void* msg, std::size_t msg_size) noexcept
{
	session* s = static_cast<session*>( tr );
	s_transport con = s->connection();
	return con->push(s->ec_, msg, msg_size);
}

ssize_t session_pull(::gnutls_transport_ptr_t tr, void* buff, std::size_t buff_size) noexcept
{
    session* s = static_cast<session*>( tr );
    s_transport con = s->connection();
    return con->pull(s->ec_, buff, buff_size);
}

// session
session::session(gnutls_session_t peer, s_transport&& connection) noexcept:
    peer_( peer ),
    connection_( std::forward<s_transport>(connection) ),
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
    //::gnutls_session_enable_compatibility_mode(peer);
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
    synch_transport *connection = new (std::nothrow) synch_transport( std::forward<s_read_write_channel>(raw) );
    if(nullptr ==  connection) {
        ec = std::make_error_code(std::errc::not_enough_memory);
        return s_session();
    }
    session *ret = new (std::nothrow) session(peer, s_transport( connection ) );
    if( nullptr == ret ) {
        ec = std::make_error_code(std::errc::not_enough_memory);
        return s_session();
    }
    ::gnutls_transport_set_ptr(peer, static_cast<::gnutls_transport_ptr_t>( ret ) );
    //::gnutls_transport_set_ptr2(peer, session_pull, session_push);
	::gnutls_transport_set_push_function(peer, session_push );
	::gnutls_transport_set_pull_function(peer, session_pull);

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


} // namespace tls

} //namespace net

} // namespace io
