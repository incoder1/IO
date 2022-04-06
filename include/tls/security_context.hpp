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
#ifndef __IO_TLS_SECURITY_CONTEXT_HPP_INCLUDED__
#define __IO_TLS_SECURITY_CONTEXT_HPP_INCLUDED__

#include "config.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include "blocking_channel.hpp"
#include "credentials.hpp"
#include "network.hpp"

namespace io {

namespace net {

namespace tls {

class security_context;
DECLARE_IPTR(security_context);

/// !\brief Transport layer security (TLS) context used to manage TLS connections
class IO_PUBLIC_SYMBOL security_context final:public io::object {
	security_context(const security_context&) = delete;
	security_context& operator=(const security_context&) = delete;
public:

    static s_security_context create(std::error_code& ec,const s_io_context& ioc) noexcept;

    ~security_context() noexcept;

    /// Opens secured client blocking connection for a TCP channel
    /// \param ec operation error code
    /// \param skt a TCP socket
    /// \return read write channel to communicate over the TLS protected TCP
    io::s_read_write_channel client_blocking_connect(std::error_code& ec, socket&& skt) const noexcept;

    /// Opens secured client blocking connection for a TCP channel
	/// \param ec operation error code
    /// \param host a TCP server host to connect to
    /// \param port a TCP server port
    /// \return read write channel to communicate over the TLS protected TCP
    io::s_read_write_channel client_blocking_connect(std::error_code& ec,const char* host, uint16_t port) const noexcept;

/*
	/// Opens non blocking client asynchronous connection to network socket
    /// \param ec operation error code, contains error in case of a network issue or attempt to connect more then once
    /// \param aioc client asynchronous io context instance
    /// \param socket a socket object to connect
    /// \param routine asynchronous IO operations completion routine
    /// \return new read/write channel object smart pointer which can be used to communicate over the network, or empty smart pointer in case of error
    s_asynch_channel client_asynch_connect(std::error_code& ec, const s_asynch_io_context& aioc, socket&& socket,const s_asynch_completion_routine& routine) const noexcept;
*/
private:
    security_context(const io::s_io_context& ioc, credentials&& creds) noexcept;
private:
    io::s_io_context ioc_;
    credentials creds_;
};

}  // namespace tls

} //namespace net

} // namespace io

#endif // __IO_TLS_SECURITY_CONTEXT_HPP_INCLUDED__
