#ifndef __IO_WIN_IO_CONTEXT_HPP_INCLUDED__
#define __IO_WIN_IO_CONTEXT_HPP_INCLUDED__

#include <config.hpp>
#include "winconf.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <channels.hpp>
#include <buffer.hpp>

#include "handlechannel.hpp"
#include "sockets.hpp"

namespace io
{

namespace win
{

class thread_pool;
DECLARE_IPTR(thread_pool);

class thread_pool:public io::object
{
    thread_pool(const thread_pool&) = delete;
    thread_pool operator=(const thread_pool&) = delete;
public:
    static s_thread_pool create(std::error_code& ec, ::DWORD max_threads) noexcept;

    inline ::DWORD max_threads() noexcept
    {
        return max_threads_;
    }

    virtual ~thread_pool() noexcept;

private:

    explicit thread_pool(::PTP_POOL id, ::DWORD max_threads, ::PTP_CLEANUP_GROUP cleanup_group) noexcept;

private:
    ::PTP_POOL id_;
    ::DWORD max_threads_;
    ::PTP_CLEANUP_GROUP cleanup_group_;
};


} // namespace win

/// !brief Asynchronous input/output context
class IO_PUBLIC_SYMBOL io_context final:public io::object
{
    io_context(const io_context&) = delete;
    io_context& operator=(const io_context&) = delete;
public:
    static s_io_context create(std::error_code& ec) noexcept;
    virtual ~io_context() noexcept override;

	/// Opens bloking connection to network socket
	/// \param ec operation error code, contains error in case of a network issue or attempt to connect more then once
	/// \param socket a socket object to connect
	/// \return new read/write channel object smart pointer which can be used to communicate over the network, or empty smart pointer in case of error
	s_read_write_channel connect(std::error_code& ec, net::socket&& socket) noexcept;

private:
//    // make socket asynchronous
//    void bind(std::error_code& ec,const s_asynch_read_channel& src) const noexcept;
//    void bind(std::error_code& ec,const s_asynch_write_channel& src) const noexcept;
    void bind(std::error_code& ec,const s_asynch_channel& src) const noexcept;

	static void listen_routine(::HANDLE port) noexcept;
    io_context(::HANDLE port, win::s_thread_pool&& tp) noexcept;
 private:
    ::HANDLE port_;
    win::s_thread_pool tp_;
    friend class nobadalloc<io_context>;
};

} // namespace io

#endif // __IO_WIN_IO_CONTEXT_HPP_INCLUDED__
