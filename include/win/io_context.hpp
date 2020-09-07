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
#include "criticalsection.hpp"
#include "sockets.hpp"
#include "synch_socket_channel.hpp"
#include "thread_pool.hpp"

namespace io {

namespace detail {

class asynch_io_context final {
    asynch_io_context(const asynch_io_context&) = delete;
    asynch_io_context& operator=(const asynch_io_context&) = delete;
public:
    ~asynch_io_context() noexcept;
    // start a
    bool bind_to_port(std::error_code& ec,const asynch_channel* src) const noexcept;
    void await() const noexcept;
 	// Notify all workers they should stop waiting for new complation events
	void shutdown(std::error_code& ec) const noexcept;
    static asynch_io_context* create(std::error_code& ec) noexcept;
    explicit asynch_io_context(::HANDLE ioc_port, io::s_thread_pool&& thread_poll) noexcept;
private:
    static void completion_loop_routine(::HANDLE ioc_port) noexcept;
private:
    ::HANDLE ioc_port_;
    io::s_thread_pool workers_;
};

} // namespace detail

/// !brief Asynchronous input/output context
class IO_PUBLIC_SYMBOL io_context final:public io::object {
    io_context(const io_context&) = delete;
    io_context& operator=(const io_context&) = delete;
public:
    static s_io_context create(std::error_code& ec) noexcept;
    virtual ~io_context() noexcept override;

    /// Opens blocking client connection to network socket
    /// \param ec operation error code, contains error in case of a network issue or attempt to connect more then once
    /// \param socket a socket object to connect
    /// \return new read/write channel object smart pointer which can be used to communicate over the network, or empty smart pointer in case of error
    s_read_write_channel client_blocking_connect(std::error_code& ec, net::socket&& socket) const noexcept;

    /// Opens non blocking client asynchronous connection to network socket
    /// \param ec operation error code, contains error in case of a network issue or attempt to connect more then once
    /// \param socket a socket object to connect
    /// \param routine asynchronous IO operations completion routine
    /// \return new read/write channel object smart pointer which can be used to communicate over the network, or empty smart pointer in case of error
    s_asynch_channel client_asynch_connect(std::error_code& ec, net::socket&& socket,const s_asynch_completion_routine& routine) const noexcept;

	/// Notify asynchronous completion workers, that no longer asynchronous io expected
	/// \param ec operation error code
    void shutdown_asynchronous(std::error_code& ec) const;

    /// Await all asynchronous operations to finish
    void await_asynchronous(std::error_code& ec);

private:
    io_context() noexcept;
    ::SOCKET new_scoket(std::error_code& ec, net::ip_family af, net::transport prot, bool asynch) const noexcept;
    detail::asynch_io_context* asynch_context(std::error_code& ec) const noexcept;
    friend class nobadalloc<io_context>;
private:
    mutable std::atomic<detail::asynch_io_context*> asynch_context_;
    mutable critical_section mtx_;
};

} // namespace io

#endif // __IO_WIN_IO_CONTEXT_HPP_INCLUDED__
