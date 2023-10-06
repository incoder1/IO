/*
 *
 * Copyright (c) 2016-2022
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#include "stdafx.hpp"
#include "io/net/win/io_context.hpp"

namespace io {

static ::SOCKET new_socket(std::error_code& ec, net::ip_family af, net::transport prot, bool asynch) noexcept
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

// io_context
s_io_context io_context::create(std::error_code& ec) noexcept
{
	io_context *ret = nobadalloc<io_context>::construct(ec);
	return ec ? s_io_context() : s_io_context(ret);
}


io_context::io_context() noexcept:
	io::object()
{
}

io_context::~io_context() noexcept
{}

s_read_write_channel io_context::client_blocking_connect(std::error_code& ec, net::socket&& socket) const noexcept
{
	s_read_write_channel ret;
	::SOCKET s = new_socket(ec, socket.get_endpoint().family(), socket.transport_protocol(), false );
	if(!ec) {
		const ::addrinfo *ai = static_cast<const ::addrinfo *>(socket.get_endpoint().native());
		sockaddr* addr = ai->ai_addr;
		int addrlen = static_cast<int>(ai->ai_addrlen);
		if(SOCKET_ERROR == ::WSAConnect(s, addr, addrlen, nullptr,nullptr,nullptr,nullptr) ) {
			ec = net::make_wsa_last_error_code();
			::closesocket(s);
		}
		else {
			net::synch_socket_channel *raw = new (std::nothrow) net::synch_socket_channel(s);
			if(nullptr == raw)
				ec = std::make_error_code(std::errc::not_enough_memory);
			else
				ret.reset(raw, true);
		}
	}
	return ret;
}

s_read_write_channel io_context::client_blocking_connect(std::error_code& ec, const char* host, uint16_t port) const noexcept
{
	const io::net::socket_factory *sf = io::net::socket_factory::instance(ec);
	if(!ec) {
		return client_blocking_connect(ec, sf->client_tcp_socket(ec,host,port) );
	}
	return s_read_write_channel();
}

// aynch_io_context
s_asynch_io_context asynch_io_context::create(std::error_code& ec, const s_io_context& owner) noexcept
{
	// Detect system logical CPUS, to calculate needed worker threads, according to D. Richer "Windows for C++"
	::SYSTEM_INFO sysinfo;
	::GetSystemInfo(&sysinfo);
	const ::DWORD max_worker_threads = (sysinfo.dwNumberOfProcessors * 2);
	// Create Windows IO completion port kernel abstraction
	::HANDLE ioc_port = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, max_worker_threads);
	s_asynch_io_context ret;
	if( nullptr == ioc_port) {
		ec = std::error_code( ::GetLastError(), std::system_category() );
	}
	else {
		// Create workers thread pool, all of them will reach same routine handling asynch io operation ending
		io::s_thread_pool workers = io::thread_pool::create(ec,  max_worker_threads );
		if(!ec) {
			asynch_io_context* px = new (std::nothrow) asynch_io_context(ioc_port, std::move(workers), owner);
			if(nullptr == px)
				ec = std::make_error_code(std::errc::not_enough_memory);
			else
				ret.reset(px,true);
		}
	}
	return ret;
}

asynch_io_context::asynch_io_context(::HANDLE ioc_port, s_thread_pool&& workers, const s_io_context& owner) noexcept:
	io::object(),
	ioc_port_(ioc_port),
	workers_( std::forward<s_thread_pool>(workers) ),
	owner_(owner)
{
	// set-up all workers threads
	std::error_code ec;
	for(unsigned int i=0; i < workers_->max_threads(); i++ ) {
		workers_->sumbmit( ec, std::bind(&asynch_io_context::completion_loop_routine, ioc_port) );
	}
}

asynch_io_context::~asynch_io_context() noexcept
{
	::CloseHandle(ioc_port_);
}


void asynch_io_context::shutdown() const noexcept
{
	// notify handler threads to stop
	for(unsigned int i=0; i < workers_->max_threads(); i++ ) {
		::PostQueuedCompletionStatus(ioc_port_, 0, 0, nullptr);
	}
}

void asynch_io_context::notify_send(std::error_code& ec,::DWORD transfered,asynch_channel* channel, io::byte_buffer&& data) noexcept
{
	// Ref count done previesly
	io::s_asynch_channel ach(channel, false);
	data.shift(transfered);
	ach->routine()->sent( ec, ach, std::forward<io::byte_buffer>(data) );
}

void asynch_io_context::notify_received(std::error_code& ec,::DWORD transfered,asynch_channel* channel, io::byte_buffer&& data) noexcept
{
	data.move(transfered);
	data.flip();
	// Ref count done previesly
	io::s_asynch_channel ach(channel, false);
	ach->routine()->received( ec, ach, std::forward<io::byte_buffer>(data) );
}

/// this routine hands all asynchronous input/output operations completion
/// used as main routine function by all worker thread pool threads
void asynch_io_context::completion_loop_routine(::HANDLE ioc_port) noexcept
{
	::BOOL status = FALSE;
	asynch_channel* channel = nullptr;
	do {
		::DWORD transfered = 0;
		win::overlapped* ovlp;
		status = ::GetQueuedCompletionStatus(
					 ioc_port,
					 &transfered,
					 reinterpret_cast<::PULONG_PTR>(&channel),
					 reinterpret_cast<::LPOVERLAPPED*>(&ovlp),
					 INFINITE);
		std::unique_ptr<win::overlapped, decltype([] (win::overlapped*px) {
			px->~overlapped();
			io::memory_traits::free(px);
		})> context( ovlp );
		if(TRUE == status && nullptr != channel) {
			std::error_code ec;
			// don't increase reference counting, must be done by channel in a thread requested asynch io operation
			switch(ovlp->io_op_) {
			case io::win::operation::send:
				notify_send( ec, transfered,  channel, std::move(context->data_) );
				break;
			case io::win::operation::recaive:
				notify_received(ec, transfered, channel, std::move(context->data_) );
				break;
			case io::win::operation::accept:
				// TODO: implement
				break;
			}
		}
	}
	while(TRUE != status || nullptr != channel);
}

/// binds a device asynchronous channel (file, pipe or socket) to io completion port
/// so that worker threads able to process input/output operations
bool asynch_io_context::bind_to_port(std::error_code& ec, const asynch_channel* src) const noexcept
{
	const io::win::win_asynch_channel  *ch = reinterpret_cast<const io::win::win_asynch_channel*>(src);
	bool ret = true;
	if( ioc_port_ != ::CreateIoCompletionPort( ch->handle(), ioc_port_, reinterpret_cast<ULONG_PTR>( src ), 0 ) ) {
		ec.assign( ::GetLastError(), std::system_category() );
		ret = false;
	}
	return ret;
}

s_asynch_channel asynch_io_context::client_asynch_connect(std::error_code& ec, net::socket&& socket,const s_asynch_completion_routine& routine) const noexcept
{
	s_asynch_channel ret;
	::SOCKET s = new_socket(ec, socket.get_endpoint().family(), socket.transport_protocol(), true );
	if(!ec) {
		net::asynch_socket_channel *channel = new (std::nothrow) net::asynch_socket_channel(s, routine, this);
		if(nullptr == channel) {
			ec = std::make_error_code(std::errc::not_enough_memory);
		}
		else {
			ret.reset(channel);
			if( bind_to_port(ec, channel) ) {
				const ::addrinfo *ai = static_cast<const ::addrinfo *>(socket.get_endpoint().native());
				sockaddr* addr = ai->ai_addr;
				int addrlen = static_cast<int>(ai->ai_addrlen);
				if(SOCKET_ERROR == ::WSAConnect(s, addr, addrlen, nullptr,nullptr,nullptr,nullptr) )
					ec = net::make_wsa_last_error_code();
			}
		}
	}
	return ret;
}

void asynch_io_context::await() const noexcept
{
	// Wait all workers for finishing
	workers_->join();
}

s_read_write_channel asynch_io_context::client_blocking_connect(std::error_code& ec, net::socket&& socket) const noexcept
{
	return owner_->client_blocking_connect(ec, std::forward<net::socket>(socket) );
}

s_read_write_channel asynch_io_context::client_blocking_connect(std::error_code& ec, const char* host, uint16_t port) const noexcept
{
	return owner_->client_blocking_connect(ec, host, port );
}

} // namespace io

