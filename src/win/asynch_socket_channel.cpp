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
#include "stdafx.hpp"
#include "asynch_socket_channel.hpp"

namespace io {

namespace net {

asynch_socket_channel::asynch_socket_channel(::SOCKET socket,const s_asynch_completion_routine& routine, const io::io_context* context) noexcept:
    asynch_channel( socket_id(socket), routine, context)
{
}

asynch_socket_channel::~asynch_socket_channel() noexcept
{
    ::closesocket(native());
}

static char* cstrcast_cast(const uint8_t* px) noexcept
{
    return const_cast<char*>(reinterpret_cast<const char*>(px));
}

void asynch_socket_channel::read(uint8_t* into, std::size_t limit, std::size_t start_from) const noexcept
{
    ::WSABUF wsab = { static_cast<::DWORD>(limit), cstrcast_cast(into) };
    detail::overlapped *ovlp = new (std::nothrow) detail::overlapped(detail::operation::recaive,into, start_from);
    ::DWORD flags = 0;
    ::WSARecv(native(), &wsab, 1, nullptr, &flags, reinterpret_cast<LPWSAOVERLAPPED>(ovlp), nullptr);
}

void asynch_socket_channel::write(const uint8_t* what, std::size_t bytes, std::size_t start_from) const noexcept
{
    ::WSABUF wsab = { static_cast<::DWORD>(bytes), cstrcast_cast(what) };
    detail::overlapped *ovlp = new (std::nothrow) detail::overlapped(detail::operation::send,const_cast<uint8_t*>(what), start_from);
    ::DWORD flags = 0;
    ::DWORD sent;
    ::WSASend(native(), &wsab, 1, &sent, flags, reinterpret_cast<LPWSAOVERLAPPED>(ovlp), nullptr);
}

bool asynch_socket_channel::cancel_pending() const noexcept
{
	return false;
}


bool asynch_socket_channel::cancel_all() const noexcept
{
    return false;
}

} // namespace net

} // namespace io
