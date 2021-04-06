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
#include "security_context.hpp"

namespace io {

namespace net {

namespace tls {

// security_context
s_security_context security_context::create(std::error_code& ec,const io::s_io_context& ioc) noexcept
{
    if( GNUTLS_E_SUCCESS == ::gnutls_global_init() ) {
#ifndef NDEBUG
			gnutls_global_set_log_level(2);
				::gnutls_global_set_log_function([] (int e, const char *msg) {
					std::fprintf(stderr, "%i %s", e, msg);
				});
#endif // NDEBUG
        credentials xcred = credentials::system_trust_creds(ec);
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
    s_read_write_channel raw = ioc_->client_blocking_connect(ec, std::forward<io::net::socket>(socket) );
    if( !ec ) {
		s_session clnts = session::client_blocking_session(ec, creds_.get(),  std::move(raw) );
        if( !ec ) {
			blocking_channel *ret = new (std::nothrow) blocking_channel( std::move(clnts) );
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

//s_asynch_channel security_context::client_asynch_connect(std::error_code& ec, const s_asynch_io_context& aioc, net::socket&& socket,const s_asynch_completion_routine& routine) const noexcept
//{
//}


} // namespace tls

} // namespace net

} // namespace io
