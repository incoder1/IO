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
#include "secure_channel.hpp"
#include "threading.hpp"

#ifdef _WIN32
#   include <Wincrypt.h>
#endif // _WIN32


namespace io {

namespace net {

namespace secure {

// session

ssize_t session::push(::gnutls_transport_ptr_t t, const void * data, std::size_t size)
{
    read_write_channel *tr = static_cast<read_write_channel*>(t);
    std::error_code ec;
    std::size_t ret = tr->write( ec, static_cast<const uint8_t*>(data), size);
    return ec ?  -1 : ret;
}

ssize_t session::pull(::gnutls_transport_ptr_t t, void * data, std::size_t max_size)
{
    read_write_channel *tr = static_cast<read_write_channel*>(t);
    std::error_code ec;
    std::size_t ret = tr->read( ec, static_cast<uint8_t*>(data), max_size);
    return ec ?  -1 : ret;
}

int session::client_handshake(::gnutls_session_t const peer) noexcept
{
    // ::gnutls_handshake_set_timeout(peer, GNUTLS_DEFAULT_HANDSHAKE_TIMEOUT);
    // ::gnutls_handshake_set_timeout(peer, GNUTLS_INDEFINITE_TIMEOUT);
    int err;
    do {
        err = ::gnutls_handshake(peer);
    } while (err < 0 &&  0 == ::gnutls_error_is_fatal(err) );
    return err;
}

session session::client_session(std::error_code &ec, ::gnutls_certificate_credentials_t crd,s_read_write_channel&& socket) noexcept
{
    session ret( std::forward<s_read_write_channel>(socket) );
    int err = ::gnutls_init( &ret.peer_, GNUTLS_CLIENT);
    if(err > 0)
        ec = std::make_error_code( std::errc::broken_pipe );
    err = ::gnutls_set_default_priority(ret.peer_);
    if(err > 0 )
        ec = std::make_error_code( std::errc::broken_pipe );

    err = ::gnutls_credentials_set(ret.peer_, GNUTLS_CRD_CERTIFICATE, crd);
    if(err > 0 )
        ec = std::make_error_code( std::errc::broken_pipe );

    ::gnutls_transport_ptr_t transport = reinterpret_cast<gnutls_transport_ptr_t>( ret.socket_.get() );

    ::gnutls_transport_set_ptr( ret.peer_, transport );
    ::gnutls_transport_set_push_function( ret.peer_, &session::push );
    ::gnutls_transport_set_pull_function( ret.peer_, &session::pull );
    //  ::gnutls_transport_set_pull_timeout_function( ret.peer_, [](gnutls_transport_ptr_t t, unsigned int ms) {
    //          ::gnutls_transport_set_errno(ret.peer_, EINTR );
    //  } );

    err = client_handshake( ret.peer_ );
    switch( err ) {
    case GNUTLS_E_SUCCESS:
        break;
    case GNUTLS_E_CERTIFICATE_VERIFICATION_ERROR:
        //std::printf("Certificate check failed\n");
        ec = std::make_error_code( std::errc::connection_refused );
        break;
    case GNUTLS_E_FATAL_ALERT_RECEIVED:
        //std::printf("fatal allert: %s\n", ::gnutls_alert_get_name( ::gnutls_alert_get(ret.peer_) ) );
        ec = std::make_error_code( std::errc::connection_refused );
        break;
    default:
        ec = std::make_error_code( std::errc::connection_refused );
    }
    // std::printf("- Session info: %s\n", ::gnutls_session_get_desc(ret.peer_) );
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
        ec  = std::make_error_code( std::errc::broken_pipe );
        return 0;
    }
    return static_cast<std::size_t>( ret );
}

std::size_t session::write(std::error_code& ec, const uint8_t *data, std::size_t data_size) const noexcept
{
    ssize_t ret = ::gnutls_record_send(peer_, static_cast<const void*>(data), data_size);
    if( ret < 0 ) {
        ec  = std::make_error_code( std::errc::broken_pipe );
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
            if( GNUTLS_E_SUCCESS != ::gnutls_global_init() ) {
                ec = std::make_error_code(std::errc::io_error);
                return nullptr;
            }
            credentials xcred = credentials::system_trust_creds(ec);
            if(!ec) {
                std::atexit( &service::destroy_gnu_tls_atexit );
                ret = new (std::nothrow) service( std::move(xcred) );
                if(nullptr == ret)
                    ec = std::make_error_code(std::errc::not_enough_memory);
            }
            _instance.store(ret, std::memory_order_release);
        }
    }
    return ret;
}

service::service(credentials&& creds) noexcept:
    creds_( std::forward<credentials>(creds) )
{}

service::~service() noexcept
{}

s_read_write_channel service::new_client_connection(std::error_code& ec, const s_read_write_channel& socket) const noexcept
{

    session s = session::client_session(ec, creds_.get(),  s_read_write_channel( socket ) );
    if( ec )
        return s_read_write_channel();
    tls_channel* ch = nobadalloc< tls_channel >::construct(ec, std::move(s) );
    return ec ? s_read_write_channel() : s_read_write_channel( ch );
}


} // namespace secure

} // namespace net

} // namespace io
