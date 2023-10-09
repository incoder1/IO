/*
 *
 * Copyright (c) 2016-2023
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#include "stdafx.hpp"
#include "io/net/win/asynch_socket_channel.hpp"

namespace io {

namespace net {

asynch_socket_channel::asynch_socket_channel(::SOCKET socket,const s_asynch_completion_routine& routine, const asynch_io_context* context) noexcept:
	win::win_asynch_channel( socket_id(socket), routine, context)
{
}

asynch_socket_channel::~asynch_socket_channel() noexcept
{
	::closesocket(native());
}

void asynch_socket_channel::recaive(std::error_code& ec, byte_buffer&& to, std::size_t position) const noexcept
{
	if(!ec) {
		::WSABUF wsab[1];
		wsab[0].len = static_cast<::DWORD>(to.available());
		wsab[0].buf = const_cast<char*>(to.position().cdata());
		win::overlapped* ovlpd = io::memory_traits::malloc_array<win::overlapped>(1);
		if(nullptr == ovlpd) {
			ec = std::make_error_code(std::errc::not_enough_memory);
		}
		else {
			ovlpd = new ( static_cast<void*>(ovlpd) ) win::overlapped(
				win::operation::recaive,
				std::forward<byte_buffer>(to),
				position);
			::DWORD flags = 0;
			int errc = ::WSARecv(native(), wsab, 1, nullptr, &flags, ovlpd, nullptr);
			if(SOCKET_ERROR == errc && WSA_IO_PENDING != ::WSAGetLastError() ) {
				ovlpd->~overlapped();
				memory_traits::free(ovlpd);
				ec = make_wsa_last_error_code();
			}
			else  {
				// don't allow to release smart pointer by this thread, until pending recaive operation
				intrusive_ptr_add_ref( const_cast<asynch_socket_channel*>(this) );
			}
		}
	}
}

void asynch_socket_channel::send(std::error_code& ec, byte_buffer&& what,std::size_t position) const noexcept
{
	::WSABUF wsab[1];
	const ::DWORD bufs = 1;
	wsab[0].buf = const_cast<char*>(what.position().cdata());
	wsab[0].len = static_cast<::DWORD>(what.length());
	win::overlapped* ovlpd = io::memory_traits::malloc_array<win::overlapped>(1);
	if(nullptr == ovlpd) {
		ec = std::make_error_code(std::errc::not_enough_memory);
	}
	else {
		ovlpd = new ( static_cast<void*>(ovlpd) ) win::overlapped(
			win::operation::send,
			std::forward<io::byte_buffer>(what),
			position
		);
		::DWORD flags = 0;
		if(SOCKET_ERROR == ::WSASend(native(), wsab, bufs, nullptr, flags, ovlpd, nullptr) ) {
			ovlpd->~overlapped();
			memory_traits::free(ovlpd);
			ec = make_wsa_last_error_code();
		} else {
			// don't allow to release smart pointer by this thread, until pending send operation
			intrusive_ptr_add_ref( const_cast<asynch_socket_channel*>(this) );
		}
	}
}

bool asynch_socket_channel::cancel_pending() const noexcept
{
	win::overlapped *ovlpd = nullptr;
	BOOL ret = ::CancelIoEx(handle(), ovlpd);
	if(nullptr != ovlpd) {
		ovlpd->~overlapped();
		memory_traits::free( ovlpd );
	}
	return TRUE == ret;
}


bool asynch_socket_channel::cancel_all() const noexcept
{
	return TRUE == ::CancelIo(handle());
}

} // namespace net

} // namespace io
