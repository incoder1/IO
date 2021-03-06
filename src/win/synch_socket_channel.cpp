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
#include "synch_socket_channel.hpp"

#undef send
#undef recv

namespace io {

namespace net {

synch_socket_channel::synch_socket_channel(::SOCKET socket) noexcept:
	read_write_channel(),
	socket_(socket)
{}

synch_socket_channel::~synch_socket_channel() noexcept
{
	::closesocket(socket_);
}

static char* cstrcast_cast(const uint8_t* px) noexcept {
	return const_cast<char*>(reinterpret_cast<const char*>(px));
}

std::size_t synch_socket_channel::read(std::error_code& ec,uint8_t* const buff, std::size_t bytes) const noexcept
{
	::WSABUF wsab;
	wsab.len = static_cast<::u_long>(bytes);
	wsab.buf = cstrcast_cast(buff);
	::DWORD ret = 0, flags = 0;
	if(SOCKET_ERROR == ::WSARecv(socket_, &wsab, 1, &ret, &flags, nullptr, nullptr) ) {
		ec = make_wsa_last_error_code();
		return 0;
	}
	return static_cast<::std::size_t>(ret);
}

std::size_t synch_socket_channel::write(std::error_code& ec, const uint8_t* buff,std::size_t size) const noexcept
{
	::WSABUF wsab;
	wsab.len = static_cast<::u_long>(size);
	wsab.buf = cstrcast_cast(buff);
	::DWORD ret = 0;
	if(SOCKET_ERROR == ::WSASend(socket_, &wsab, 1, &ret, 0, nullptr, nullptr) ) {
		ec = make_wsa_last_error_code();
		return 0;
	}
	return static_cast<::std::size_t>(ret);
}


} // namespace net

} // namespace io

