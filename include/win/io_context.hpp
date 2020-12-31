/*
 *
 * Copyright (c) 2016-2020
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef __IO_WIN_IO_CONTEXT_HPP_INCLUDED__
#define __IO_WIN_IO_CONTEXT_HPP_INCLUDED__

#include <config.hpp>
#include "winconf.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <channels.hpp>
#include <buffer.hpp>

#include "asynch_socket_channel.hpp"
#include "sockets.hpp"
#include "synch_socket_channel.hpp"
#include "thread_pool.hpp"

namespace io {

/// !brief Asynchronous input/output context
class IO_PUBLIC_SYMBOL io_context final:public io::object {
    io_context(const io_context&) = delete;
    io_context& operator=(const io_context&) = delete;
public:
	/**
	 * Creates new io
	 */
    static s_io_context create(std::error_code& ec) noexcept;

    virtual ~io_context() noexcept override;

    /// Opens blocking client connection to network socket
    /// \param ec operation error code, contains error in case of a network issue or attempt to connect more then once
    /// \param socket a socket object to connect
    /// \return new read/write channel object smart pointer which can be used to communicate over the network, or empty smart pointer in case of error
    s_read_write_channel client_blocking_connect(std::error_code& ec, net::socket&& socket) const noexcept;

	/// Opens blocking client connection to network address
    /// \param ec operation error code, contains error in case of a network issue or attempt to connect more then once
	/// \param host host name text representation, i.e. DNS name or IP address (V4 or V6)
	/// \param port network port
    /// \return new read/write channel object smart pointer which can be used to communicate over the network, or empty smart pointer in case of error
    s_read_write_channel client_blocking_connect(std::error_code& ec, const char* host, uint16_t port) const noexcept;


private:
    io_context() noexcept;
    friend class nobadalloc<io_context>;
};

class asynch_io_context final:public io::object {
    asynch_io_context(const asynch_io_context&) = delete;
    asynch_io_context& operator=(const asynch_io_context&) = delete;
public:
 	/// Creates new asynchronous input/output context
	/// \param ec operation error code
	/// \param owner an exising synchronous io_context
	/// \return a smart pointer on reference to new asynch_io_context or an empty smart pointer in case of error
	static s_asynch_io_context create(std::error_code& ec, const s_io_context& owner) noexcept;

	/// Creates new asynchronous input/output context
	/// \param ec operation error code
	/// \return a smart pointer on reference to new asynch_io_context or an empty smart pointer in case of error
	static inline s_asynch_io_context create(std::error_code& ec) noexcept
	{
		s_io_context ctx = io::io_context::create(ec);
		return ec ? s_asynch_io_context() : create(ec, ctx);
	}


    ~asynch_io_context() noexcept;

	/// Opens blocking client connection to network socket
    /// \param ec operation error code, contains error in case of a network issue or attempt to connect more then once
    /// \param socket a socket object to connect
    /// \return new read/write channel object smart pointer which can be used to communicate over the network, or empty smart pointer in case of error
    s_read_write_channel client_blocking_connect(std::error_code& ec, net::socket&& socket) const noexcept;
	/// Opens blocking client connection to network address
    /// \param ec operation error code, contains error in case of a network issue or attempt to connect more then once
	/// \param host host name text representation, i.e. DNS name or IP address (V4 or V6)
	/// \param port network port
    /// \return new read/write channel object smart pointer which can be used to communicate over the network, or empty smart pointer in case of error
    s_read_write_channel client_blocking_connect(std::error_code& ec, const char* host, uint16_t port) const noexcept;

	/// Opens non blocking client asynchronous connection to network socket
    /// \param ec operation error code, contains error in case of a network issue or attempt to connect more then once
    /// \param socket a socket object to connect
    /// \param routine asynchronous IO operations completion routine
    /// \return new read/write channel object smart pointer which can be used to communicate over the network, or empty smart pointer in case of error
    s_asynch_channel client_asynch_connect(std::error_code& ec, net::socket&& socket,const s_asynch_completion_routine& routine) const noexcept;

    /// Waits until all asynchronous operations done, including shutdown signal
    void await() const noexcept;

 	/// Notify all workers they should stop waiting for new comletion events, and release waiting threads
	void shutdown() const noexcept;


private:
    static void notify_send(std::error_code& ec,::DWORD transfered,asynch_channel* channel,io::byte_buffer&& data) noexcept;
	static void notify_received(std::error_code& ec,::DWORD transfered,asynch_channel* channel, io::byte_buffer&& data) noexcept;
    static void completion_loop_routine(::HANDLE ioc_port) noexcept;

    asynch_io_context(::HANDLE ioc_port, io::s_thread_pool&& thread_poll, const s_io_context& owner) noexcept;
    bool bind_to_port(std::error_code& ec,const asynch_channel* src) const noexcept;

private:
    ::HANDLE ioc_port_;
    io::s_thread_pool workers_;
	s_io_context owner_;
};

} // namespace io

#endif // __IO_WIN_IO_CONTEXT_HPP_INCLUDED__
