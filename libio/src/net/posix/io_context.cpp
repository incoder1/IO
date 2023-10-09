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
#include "io/net/posix/io_context.hpp"

namespace io {

namespace net {

static constexpr int SOCKET_ERROR = -1;
static constexpr int INVALID_SOCKET = -1;

static int new_socket(std::error_code& ec, int af, transport prot) noexcept
{
	int type = 0;
	switch(prot) {
	case transport::tcp:
		type = SOCK_STREAM;
		break;
	case transport::udp:
		type = SOCK_DGRAM;
		break;
	case transport::icmp:
	case transport::icmp6:
		type = SOCK_RAW;
		break;
	}
	int ret = ::socket(af,type,static_cast<int>(prot));
	if(INVALID_SOCKET != ret) {
		if(AF_INET6 == af) {
			int off = 0;
			::setsockopt(
				ret,
				IPPROTO_IPV6,
				IPV6_V6ONLY,
				reinterpret_cast<const char*>(&off),
				sizeof(off)
			);
		}
	} else {
		ec = std::error_code( errno, std::system_category() );
	}
	return ret;
}

} // namespace net

// io_context
s_io_context io_context::create(std::error_code& ec) noexcept
{
	io_context *ret = nobadalloc<io_context>::construct(ec);
	return ec ? s_io_context() : s_io_context(ret);
}

io_context::io_context() noexcept:
	io::object()
{}

io_context::~io_context() noexcept
{}

s_read_write_channel io_context::client_blocking_connect(std::error_code& ec, net::socket&& socket) const noexcept
{
	int s = io::net::new_socket(ec, static_cast<int>(socket.get_endpoint().family()), socket.transport_protocol());
	if(ec)
		return s_read_write_channel();
	const ::addrinfo *ai = static_cast<const ::addrinfo *>(socket.get_endpoint().native());
	if( net::SOCKET_ERROR == ::connect(s, ai->ai_addr, ai->ai_addrlen) ) {
		ec = std::error_code( errno,  std::system_category() );
		return s_read_write_channel();
	}
	return s_read_write_channel( nobadalloc<net::synch_socket_channel>::construct(ec, s ) );
}

s_read_write_channel io_context::client_blocking_connect(std::error_code& ec, const char* host, uint16_t port) const noexcept
{
	const io::net::socket_factory *sf = io::net::socket_factory::instance(ec);
	if(!ec) {
		return client_blocking_connect(ec, sf->client_tcp_socket(ec,host,port) );
	}
	return s_read_write_channel();
}

// demultiplexor
namespace detail {

//#ifdef __IO_EPOLL_MULTIPLEX__
s_demultiplexor demultiplexor::create(std::error_code& ec) noexcept
{
	int descriptor = ::epoll_create1(0);
	if(-1 == descriptor) {
		ec.assign( errno , std::system_category() );
		return s_demultiplexor();
	}
	demultiplexor *ret = new ( std::nothrow ) demultiplexor(descriptor);
	if(nullptr == ret) {
		ec = std::make_error_code(std::errc::not_enough_memory);
		return s_demultiplexor();
	}
	return s_demultiplexor( ret );
}

demultiplexor::~demultiplexor() noexcept
{
	::close(peer_);
}

void demultiplexor::register_descriptor(std::error_code& ec, int descriptor) noexcept
{
	int flags = ::fcntl(descriptor, F_GETFL, 0);
	flags |= O_NONBLOCK;
	if(-1 == ::fcntl(descriptor, F_SETFL, flags) ) {
		ec.assign( errno , std::system_category() );
	} else {
		::epoll_event ev;
		ev.data.fd = descriptor;
		ev.events = EPOLLIN | EPOLLET;
		if(-1 == ::epoll_ctl(peer_, EPOLL_CTL_ADD, descriptor, &ev) ) {
			ec.assign( errno , std::system_category() );
		}
	}
}

//#elif defined(__IO_KQUEUE_DEMULTIPLEX__)
//
//
//s_demultiplexor demultiplexor::create(std::error_code& ec) noexcept
//{
//
//}
//
//demultiplexor::~demultiplexor() noexcept
//{
//
//}
//
//void demultiplexor::register_descriptor(std::error_code& ec, int descriptor) noexcept
//{
//
//}
//
//#endif // __IO_EPOLL_MULTIPLEX__

} // namespace detail

// asynch_io_context
s_asynch_io_context asynch_io_context::create(std::error_code& ec, const s_io_context& owner) noexcept
{
	ec = std::make_error_code(std::errc::function_not_supported);
	return s_asynch_io_context();
}

asynch_io_context::asynch_io_context(detail::s_demultiplexor reactor, s_thread_pool&& workers, const s_io_context& owner) noexcept:
	io::object(),
	reactor_(reactor),
	workers_( std::forward<s_thread_pool>(workers) ),
	owner_(owner)
{
	// set-up all workers threads
//	std::error_code ec;
//	for(unsigned int i=0; i < workers_->max_threads(); i++ ) {
//		workers_->sumbmit( ec , std::bind(&asynch_io_context::completion_loop_routine, ioc_port) );
//	}
}

asynch_io_context::~asynch_io_context() noexcept
{

}


} // namespace io
