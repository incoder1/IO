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
#include "io/tls/security_context.hpp"

namespace io {

namespace net {

namespace tls {

#ifndef NDEBUG
static void set_tls_debug_log()
{
    ::gnutls_global_set_log_level(2);
    ::gnutls_global_set_log_function(
    [] (int e, const char *msg) {
        std::fprintf(stderr, "%i %s", e, msg);
    }
    );
}
#endif // NDEBUG

// security_context
s_security_context security_context::create(std::error_code& ec,const io::s_io_context& ioc) noexcept
{
    s_security_context ret;
    if( GNUTLS_E_SUCCESS != ::gnutls_global_init() ) {
        ec = std::make_error_code(std::errc::io_error);
    }
    else {
#ifndef NDEBUG
       set_tls_debug_log();
#endif // NDEBUG
        auto xcred = credentials::system_trust_creds(ec);
        if(!ec) {
            security_context *raw = new (std::nothrow) security_context( ioc, std::move(xcred) );
            if(nullptr == raw)
                ec = std::make_error_code(std::errc::not_enough_memory);
            else
                ret = s_security_context(raw);
        }
    }
    return ret;
}

security_context::security_context(const io::s_io_context& ioc, credentials&& creds) noexcept:
    io::object(),
    ioc_(ioc),
    creds_( std::forward<credentials>(creds) )
{}

security_context::~security_context() noexcept
{
    ::gnutls_global_deinit();
}

s_read_write_channel security_context::client_blocking_connect(std::error_code& ec,socket&& socket) const noexcept
{
    s_read_write_channel ret;
    auto socket_channel = ioc_->client_blocking_connect(ec, std::forward<io::net::socket>(socket) );
    if( !ec ) {
        auto clnts = session::client_blocking_session(ec, creds_.get(),  std::move(socket_channel) );
        if( !ec ) {
            auto *px = new (std::nothrow) blocking_channel( std::move(clnts) );
            if( nullptr == px ) {
				ec = std::make_error_code(std::errc::not_enough_memory);
            } else {
				ret = s_read_write_channel( px );
            }
        }
    }
    return ret;
}

s_read_write_channel security_context::client_blocking_connect(std::error_code& ec,const char* host, uint16_t port) const noexcept
{
    const io::net::socket_factory *sf = io::net::socket_factory::instance(ec);
	return ec ? s_read_write_channel() : client_blocking_connect(ec, sf->client_tcp_socket(ec,host,port) );
}

//s_asynch_channel security_context::client_asynch_connect(std::error_code& ec, const s_asynch_io_context& aioc, net::socket&& socket,const s_asynch_completion_routine& routine) const noexcept
//{
//}


} // namespace tls

} // namespace net

} // namespace io
