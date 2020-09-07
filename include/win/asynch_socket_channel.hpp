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

#include "channels.hpp"

#include <ws2tcpip.h>

namespace io {

namespace detail {

enum class operation: ::DWORD {
   accept = 0,
   send = 1,
   recaive = 2
};

struct overlapped: public ::OVERLAPPED {
    overlapped(operation op, uint8_t* px, uint64_t position) noexcept
    {
        Internal = reinterpret_cast<ULONG_PTR>(nullptr);
        InternalHigh = reinterpret_cast<ULONG_PTR>(nullptr);
        // set-up offset
		::LARGE_INTEGER pos;
		pos.QuadPart = position;
        OffsetHigh = pos.HighPart;
        Offset = pos.LowPart;
        hEvent = static_cast<::HANDLE>(nullptr);
        io_op_ = op;
        data_ = px;
    }
    operation op() const noexcept {
    	return io_op_;
    }
    uint8_t* data() const noexcept {
    	return data_;
    }
private:
	operation io_op_;
    uint8_t *data_;
};

}  // namespace detail

namespace net {

class IO_PUBLIC_SYMBOL asynch_socket_channel final: public io::asynch_channel {
public:
    asynch_socket_channel(::SOCKET socket, const s_asynch_completion_routine& routine, const io::io_context* context) noexcept;
    virtual ~asynch_socket_channel() noexcept override;
    virtual void read(uint8_t* into, std::size_t limit, std::size_t start_from) const noexcept override;
    virtual void write(const uint8_t* what, std::size_t bytes, std::size_t start_from) const noexcept override;
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
