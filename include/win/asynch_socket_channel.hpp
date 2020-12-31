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
#ifndef __ASYNCH_WIN_SOCKET_CHANNEL_HPP_INCLUDED__
#define __ASYNCH_WIN_SOCKET_CHANNEL_HPP_INCLUDED__

#include "config.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#ifndef SECURITY_WIN32
#	define SECURITY_WIN32
#endif // SECURITY_WIN32

#include <ws2tcpip.h>

#include "wsaerror.hpp"
#include "win_asynch_channel.hpp"

namespace io {

namespace net {

class IO_PUBLIC_SYMBOL asynch_socket_channel final: public io::win::win_asynch_channel {
public:
    asynch_socket_channel(::SOCKET socket, const s_asynch_completion_routine& routine, const asynch_io_context* ctx) noexcept;
    virtual ~asynch_socket_channel() noexcept override;
	virtual void recaive(std::error_code& ec, std::size_t amout, std::size_t position) const noexcept override;
    virtual void send(std::error_code& ec, byte_buffer&& what,std::size_t position) const noexcept override;
    virtual bool cancel_pending() const noexcept override;
    virtual bool cancel_all() const noexcept override;
    ::SOCKET native() const noexcept
    {
        return reinterpret_cast<::SOCKET>(handle());
    }
};

} // namespace net

} // namespace io

#endif // __ASYNCH_WIN_SOCKET_CHANNEL_HPP_INCLUDED__
