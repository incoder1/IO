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
#include "io_context.hpp"

namespace io
{

namespace win
{

s_thread_pool thread_pool::create(std::error_code& ec, ::DWORD max_threads) noexcept
{
    ::PTP_POOL id = ::CreateThreadpool(nullptr);
    if( nullptr == id)
    {
        ec = std::error_code( ::GetLastError(), std::system_category() );
        return s_thread_pool();
    }

    if( FALSE == ::SetThreadpoolThreadMinimum(id, 1) )
    {
        ec = std::error_code( ::GetLastError(), std::system_category() );
        ::CloseThreadpool(id);
        return s_thread_pool();
    }

    ::SetThreadpoolThreadMaximum(id, max_threads );

    ::PTP_CLEANUP_GROUP cleanup_group = ::CreateThreadpoolCleanupGroup();
    if( nullptr == cleanup_group)
    {
        ec = std::error_code( ::GetLastError(), std::system_category() );
        ::CloseThreadpool(id);
        return s_thread_pool();
    }

    thread_pool *ret = new ( std::nothrow ) thread_pool(id, max_threads, cleanup_group);
    if( nullptr == ret)
    {
        ec = std::make_error_code(std::errc::not_enough_memory);
        return s_thread_pool();
    }
    return s_thread_pool(ret);
}

thread_pool::thread_pool(::PTP_POOL id, ::DWORD max_threads, ::PTP_CLEANUP_GROUP cleanup_group) noexcept:
    object(),
    id_(id),
    max_threads_(max_threads),
    cleanup_group_(cleanup_group)
{
}

thread_pool::~thread_pool() noexcept
{
    // Clean up the cleanup group.
    ::CloseThreadpoolCleanupGroup(cleanup_group_);
    // Close thread pool
    ::CloseThreadpool(id_);
}


} // namespase win

s_io_context io_context::create(std::error_code& ec) noexcept
{
    ::SYSTEM_INFO sysinfo;
    ::GetSystemInfo(&sysinfo);
    const ::DWORD max_threads = sysinfo.dwNumberOfProcessors * 2;
    ::HANDLE port = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, max_threads);
    if( nullptr == port) {
        ec = std::error_code( ::GetLastError(), std::system_category() );
        return s_io_context();
    }
    win::s_thread_pool tp = win::thread_pool::create(ec, max_threads);
    if(ec)
        return s_io_context();
    io_context *ret = nobadalloc<io_context>::construct(ec, port, std::move(tp));
    return ec ? s_io_context() : s_io_context(ret);
}

void io_context::listen_routine(::HANDLE port) noexcept
{
    ::BOOLEAN status;
    do
    {
    	::OVERLAPPED_ENTRY entry;
		::ULONG removed;
		status = ::GetQueuedCompletionStatusEx( port, &entry, 1, &removed,  INFINITE, FALSE);
        if(FALSE == status || INVALID_HANDLE_VALUE == entry.lpOverlapped->hEvent) {
            if(ERROR_ABANDONED_WAIT_0 == ::GetLastError())
                continue;
            status = FALSE;
        } else {
        	std::error_code ec;
        	io::asynch_channel *channel = reinterpret_cast<io::asynch_channel*>(entry.lpCompletionKey);
        	win::overlapped *ovlpd = reinterpret_cast<win::overlapped*>(entry.lpOverlapped);
            switch(ovlpd->io_op)
            {
            case win::operation::send:
                channel->on_write_finished(ec, ovlpd->data, entry.dwNumberOfBytesTransferred);
                break;
            case win::operation::recaive:
            	channel->on_read_finished(ec, ovlpd->data, entry.dwNumberOfBytesTransferred);
                break;
            }
            delete ovlpd;
        }
    }
    while(TRUE == status);
}

io_context::io_context(::HANDLE port, win::s_thread_pool&& tp) noexcept:
    io::object(),
    port_(port),
    tp_( std::forward<win::s_thread_pool>(tp) )
{

}

io_context::~io_context() noexcept
{
    ::CloseHandle(port_);
}


void io_context::bind(std::error_code& ec,const s_asynch_channel& src) const noexcept {
	::HANDLE res = ::CreateIoCompletionPort( src->handle(), port_, reinterpret_cast<ULONG_PTR>( src.get() ), 0 );
    if(res != port_)
        ec = std::error_code( ::GetLastError(), std::system_category() );
}

static ::SOCKET new_scoket(std::error_code& ec, net::ip_family af, net::transport prot) noexcept
{
    int type = 0;
    switch(prot) {
    case net::transport::tcp:
        type = SOCK_STREAM;
        break;
    case net::transport::udp:
        type = SOCK_DGRAM;
        break;
    case net::transport::icmp:
    case net::transport::icmp6:
        type = SOCK_RAW;
        break;
    }
    ::SOCKET ret = ::WSASocketW(
                       static_cast<int>(af),
                       type,
                       static_cast<int>(prot),
                       nullptr, 0, 0 );
    if(ret == INVALID_SOCKET)
        ec = net::make_wsa_last_error_code();
    if(net::ip_family::ip_v6 == af) {
        int off = 0;
        ::setsockopt(
            ret,
            IPPROTO_IPV6,
            IPV6_V6ONLY,
            reinterpret_cast<const char*>(&off),
            sizeof(off)
        );
    }
    return ret;
}

s_read_write_channel io_context::connect(std::error_code& ec, net::socket&& socket) noexcept
{
	::SOCKET s = new_scoket(ec, socket.get_endpoint().family(), socket.transport_protocol() );
	if(ec)
		return s_read_write_channel();
	const ::addrinfo *ai = static_cast<const ::addrinfo *>(socket.get_endpoint().native());
	if( SOCKET_ERROR == ::WSAConnect(s, ai->ai_addr, static_cast<int>(ai->ai_addrlen), nullptr,nullptr,nullptr,nullptr) ) {
		ec = net::make_wsa_last_error_code();
		return s_read_write_channel();
	}
	return s_read_write_channel( nobadalloc<net::synch_socket_channel>::construct(ec, s ) );
}

} // namespace io

