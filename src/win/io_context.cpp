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
#include "io_context.hpp"

namespace io {

namespace detail {

// aynch_io_context
asynch_io_context* asynch_io_context::create(std::error_code& ec) noexcept
{
    asynch_io_context* ret = nullptr;

    // Detect system logical CPUS, to calculate needed worker threads, according to D. Richer "Windows for C++"
    ::SYSTEM_INFO sysinfo;
    ::GetSystemInfo(&sysinfo);
    const ::DWORD max_worker_threads = (sysinfo.dwNumberOfProcessors * 2);
    // Create Windows IO completion port kernel abstraction
    ::HANDLE ioc_port = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, max_worker_threads);
    if( nullptr == ioc_port) {
        ec = std::error_code( ::GetLastError(), std::system_category() );
    } else {
    	// Create workers thread pool, all of them will reach same routine handling asynch io operation ending
		io::s_thread_pool workers = io::thread_pool::create(ec, max_worker_threads, max_worker_threads );
		if(!ec)
			ret = new (std::nothrow) asynch_io_context(ioc_port, std::move(workers) );
    }
	return ret;
}

asynch_io_context::asynch_io_context(::HANDLE ioc_port, s_thread_pool&& workers) noexcept:
    ioc_port_(ioc_port),
    workers_( std::forward<s_thread_pool>(workers) )
{
	// set-up all workers threads
	std::error_code ec;
	for(unsigned int i=0; i < workers_->max_threads(); i++ ) {
		workers_->sumbmit( ec , thread_routine(  std::bind(&asynch_io_context::completion_loop_routine, ioc_port) )  );
	}
}

asynch_io_context::~asynch_io_context() noexcept
{
	::CloseHandle(ioc_port_);
}


void asynch_io_context::shutdown(std::error_code& ec) const noexcept {
	// notify handler threads to stop
	for(unsigned int i=0; i < workers_->max_threads(); i++ ) {
		::PostQueuedCompletionStatus(ioc_port_, 0, 0, nullptr);
	}
}

/// this routine hands all asynchronous input/output operations completion
/// used as main routine function by all worker thread pool threads
void asynch_io_context::completion_loop_routine(::HANDLE ioc_port) noexcept
{
	::BOOLEAN status;
	asynch_channel* channel = nullptr;
	do {
		::DWORD transfered = 0;
		::LPOVERLAPPED ovlp;
		status = ::GetQueuedCompletionStatus(
				ioc_port,
				&transfered,
				reinterpret_cast<PULONG_PTR>(&channel),
				&ovlp,
				INFINITE);
		if(TRUE == status && nullptr != channel) {
			detail::overlapped *overlapped = reinterpret_cast<detail::overlapped *>(ovlp);
			std::error_code ec;
			io::s_asynch_channel ach(channel, true);
			switch(overlapped->op())
			{
			case io::detail::operation::send:
				channel->routine()->sent( ec, ach, overlapped->data(), transfered );
				delete ovlp;
				break;
			case io::detail::operation::recaive:
				channel->routine()->recaived( ec, ach, overlapped->data(), transfered );
				delete ovlp;
				break;
			case io::detail::operation::accept:
				// DODO: implement
				delete ovlp;
				break;
			}
		}
	} while(TRUE != true || nullptr != channel);
}

/// binds a device asynchronous channel (file, pipe or socket) to io completion port
/// so that worker threads able to process input/output operations
bool asynch_io_context::bind_to_port(std::error_code& ec, const asynch_channel* src) const noexcept
{
    bool ret = ioc_port_ == ::CreateIoCompletionPort( src->handle(), ioc_port_, reinterpret_cast<ULONG_PTR>( src ), 0 );
    if(!ret) {
        ec = std::error_code( ::GetLastError(), std::system_category() );
    }
	return ret;
}

void asynch_io_context::await() const noexcept {
	// Wait all workers for finishing
	workers_->join();
}

} // namespace detail

// io_context
s_io_context io_context::create(std::error_code& ec) noexcept
{
    io_context *ret = nobadalloc<io_context>::construct(ec);
    return ec ? s_io_context() : s_io_context(ret);
}


io_context::io_context() noexcept:
    io::object(),
    asynch_context_(nullptr),
    mtx_()
{
}

io_context::~io_context() noexcept
{
	// check that we have asynch context
	detail::asynch_io_context* asynch_context = asynch_context_.load( std::memory_order::memory_order_relaxed);
	// If we have asynch following will wait until a routine shutdown thread pool
	if(nullptr != asynch_context)
		delete asynch_context;
}


::SOCKET io_context::new_scoket(std::error_code& ec, net::ip_family af, net::transport prot, bool asynch) const noexcept
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
    ::DWORD flags = asynch ? WSA_FLAG_OVERLAPPED : 0;
    ::SOCKET ret = ::WSASocketW(
                       static_cast<int>(af),
                       type,
                       static_cast<int>(prot),
                       nullptr, 0, flags );
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

s_read_write_channel io_context::client_blocking_connect(std::error_code& ec, net::socket&& socket) const noexcept
{
    ::SOCKET s = new_scoket(ec, socket.get_endpoint().family(), socket.transport_protocol(), false );
    if(ec)
        return s_read_write_channel();
    const ::addrinfo *ai = static_cast<const ::addrinfo *>(socket.get_endpoint().native());
    if( SOCKET_ERROR == ::WSAConnect(s, ai->ai_addr, static_cast<int>(ai->ai_addrlen), nullptr,nullptr,nullptr,nullptr) ) {
        ec = net::make_wsa_last_error_code();
        return s_read_write_channel();
    }
    return s_read_write_channel( nobadalloc<net::synch_socket_channel>::construct(ec, s ) );
}

// asynch_io_context is expensive, since it is using io cplatition port and managed thread pool
// this method layzy construct it, so that it will be used only when application needs to use asynchronous input/output
detail::asynch_io_context* io_context::asynch_context(std::error_code& ec) const noexcept
{
	detail::asynch_io_context* ret = asynch_context_.load( std::memory_order::memory_order_relaxed);
 	if(nullptr == ret) {
		io::lock_guard lock(mtx_);
		// check that not initialized by another thread previsully
		ret = asynch_context_.load( std::memory_order::memory_order_acquire);
		if(nullptr == ret) {
			// pool is not yet constructed by any thread, creating new one
			ret = detail::asynch_io_context::create(ec);
			asynch_context_.store(ret, std::memory_order_release);
		}
	}
	return ret;
}

s_asynch_channel io_context::client_asynch_connect(std::error_code& ec, net::socket&& socket,const s_asynch_completion_routine& routine) const noexcept
{
    ::SOCKET s = new_scoket(ec, socket.get_endpoint().family(), socket.transport_protocol(), true );
    if(ec)
        return s_asynch_channel();
    const ::addrinfo *ai = static_cast<const ::addrinfo *>(socket.get_endpoint().native());

    s_asynch_channel ret;
	net::asynch_socket_channel *channel = new (std::nothrow) net::asynch_socket_channel(s, routine, this);
	if(nullptr == channel) {
		ec = std::make_error_code(std::errc::not_enough_memory);
	} else {
		ret.reset(channel);
		detail::asynch_io_context *acntx = asynch_context(ec);
		if(!ec) {
			acntx->bind_to_port(ec, channel);
			int errc = ::WSAConnect(s, ai->ai_addr, static_cast<int>(ai->ai_addrlen), nullptr,nullptr,nullptr,nullptr);
			if(SOCKET_ERROR == errc)
				ec = net::make_wsa_last_error_code();
		}
	}
	return ret;
}

void io_context::shutdown_asynchronous(std::error_code& ec) const
{
	detail::asynch_io_context *acntx = asynch_context(ec);
	if(!ec) {
		acntx->shutdown(ec);
	}
}

void io_context::await_asynchronous(std::error_code& ec)
{
	detail::asynch_io_context *acntx = asynch_context(ec);
	if(!ec) {
		// io::lock_guard lock(mtx_);
		acntx->await();
	}
}

} // namespace io

