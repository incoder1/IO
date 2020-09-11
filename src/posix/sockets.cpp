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
#include "sockets.hpp"

namespace io {

namespace net {

#ifdef IO_IS_LITTLE_ENDIAN

static inline uint16_t io_ntohs(uint16_t x)
{
	return io_bswap16( x );
}

static inline uint16_t io_htons(uint16_t x)
{
	return io_bswap16( x );
}

#else

static inline uint16_t io_ntohs(uint16_t x)
{
	return x;
}

static inline uint16_t io_htons(uint16_t x)
{
	return x;
}

#endif // IO_IS_LITTLE_ENDIAN

// endpoint
endpoint::endpoint(std::shared_ptr<::addrinfo>&& info) noexcept:
	addr_info_(std::forward<std::shared_ptr<::addrinfo> >(info))
{
}

endpoint::endpoint(const std::shared_ptr<::addrinfo>& info) noexcept:
	addr_info_(info)
{}


uint16_t endpoint::port() const noexcept
{
	switch( family() ) {
	case ip_family::ip_v4:
		return io_ntohs(
		           reinterpret_cast<::sockaddr_in*>(addr_info_->ai_addr)->sin_port
		       );
	case ip_family::ip_v6:
		return io_ntohs(
		           reinterpret_cast<::sockaddr_in6*>(addr_info_->ai_addr)->sin6_port
		       );
	default:
		return 0;
	}
}

void endpoint::set_port(uint16_t port) noexcept
{
	switch( family() ) {
	case ip_family::ip_v4:
		reinterpret_cast<::sockaddr_in*>(addr_info_->ai_addr)->sin_port = io_htons(port);
		break;
	case ip_family::ip_v6:
		reinterpret_cast<::sockaddr_in6*>(addr_info_->ai_addr)->sin6_port = io_htons(port);
		break;
	default:
		break;
	}
}

ip_family endpoint::family() const noexcept
{
	return static_cast<ip_family>( addr_info_->ai_family );
}

const void* endpoint::native() const noexcept
{
	return static_cast<void*>(addr_info_.get());
}

const_string endpoint::ip_address() const noexcept
{
	char tmp[INET6_ADDRSTRLEN];
	io_zerro_mem(tmp, INET6_ADDRSTRLEN);
	const char* ret = ::inet_ntop(
	                      addr_info_->ai_family,
	                      const_cast<void*>(
	                          static_cast<const void*>(addr_info_->ai_addr)
	                      ),
	                      tmp,
	                      INET6_ADDRSTRLEN);

	return nullptr != ret ? const_string(tmp) : const_string() ;
}


// socket
socket::socket() noexcept:
	object()
{}

//// synch_socket_channel
//class synch_socket_channel final:public read_write_channel {
//private:
//	static constexpr int SOCKET_ERROR = -1;
//	friend class nobadalloc<synch_socket_channel>;
//	synch_socket_channel(int socket) noexcept:
//		read_write_channel(),
//		socket_(socket)
//	{}
//public:
//	virtual ~synch_socket_channel() noexcept
//	{
//		::close(socket_);
//	}
//	virtual std::size_t read(std::error_code& ec,uint8_t* const buff, std::size_t bytes) const noexcept override
//	{
//		::ssize_t ret = ::recv(socket_, static_cast<void*>(buff), bytes, 0);
//		if(SOCKET_ERROR == ret) {
//			ec.assign( errno, std::system_category() );
//			return 0;
//		}
//		return static_cast<::std::size_t>(ret);
//	}
//	virtual std::size_t write(std::error_code& ec, const uint8_t* buff,std::size_t size) const noexcept override
//	{
//		::ssize_t ret = ::send(socket_, static_cast<const void*>(buff), size,  0);
//		if(SOCKET_ERROR ==  ret ) {
//			ec.assign( errno, std::system_category() );
//			return 0;
//		}
//		return static_cast<::std::size_t>(ret);
//	}
//public:
//	int socket_;
//};
//
//static constexpr int SOCKET_ERROR = -1;
//static constexpr int INVALID_SOCKET = -1;
//
//static int new_socket(int af, transport prot) noexcept
//{
//	int type = 0;
//	switch(prot) {
//	case transport::tcp:
//		type = SOCK_STREAM;
//		break;
//	case transport::udp:
//		type = SOCK_DGRAM;
//		break;
//	case transport::icmp:
//	case transport::icmp6:
//		type = SOCK_RAW;
//		break;
//	}
//	int ret = ::socket(af,type,static_cast<int>(prot));
//	if(INVALID_SOCKET != ret) {
//		if(AF_INET6 == af) {
//			int off = 0;
//			::setsockopt(
//			    ret,
//			    IPPROTO_IPV6,
//			    IPV6_V6ONLY,
//			    reinterpret_cast<const char*>(&off),
//			    sizeof(off)
//			);
//		}
//	}
//	return ret;
//}
//
//// intet_socket
//class inet_socket final: public socket {
//public:
//
//	inet_socket(endpoint&& ep, transport t_prot) noexcept:
//		socket(),
//		transport_(t_prot),
//		connected_(false),
//		ep_( std::forward<endpoint>(ep) )
//	{}
//
//	virtual endpoint get_endpoint() const noexcept override
//	{
//		return ep_;
//	}
//
//	virtual transport transport_protocol() const noexcept override
//	{
//		return transport_;
//	}
//
//	virtual bool connected() const noexcept override
//	{
//		return connected_.load( std::memory_order_seq_cst );
//	}
//
//	virtual s_read_write_channel connect(std::error_code& ec) const noexcept override
//	{
//		bool tmp = connected_.load( std::memory_order_relaxed );
//		if( tmp ||
//		        !connected_.compare_exchange_strong(
//		            tmp, true,
//		            std::memory_order_acquire,
//		            std::memory_order_relaxed
//		        )
//		  ) {
//			ec = std::make_error_code( std::errc::device_or_resource_busy );
//			return s_read_write_channel();
//		}
//		const ::addrinfo *ai = static_cast<const ::addrinfo *>(ep_.native());
//		int s = new_socket( ai->ai_family, transport_);
//		if(INVALID_SOCKET == s ) {
//			connected_.store(false, std::memory_order_release );
//			ec.assign( errno, std::system_category() );
//			return s_read_write_channel();
//		}
//		if(SOCKET_ERROR == ::connect(s, ai->ai_addr, ai->ai_addrlen) ) {
//			connected_.store(false, std::memory_order_release );
//			ec.assign( errno, std::system_category() );
//			return s_read_write_channel();
//		}
//		connected_.store(true, std::memory_order_release );
//		return s_read_write_channel( nobadalloc<synch_socket_channel>::construct(ec, s ) );
//	}
//
//private:
//	transport transport_;
//	mutable std::atomic_bool connected_;
//	endpoint ep_;
//};

// socket_factory
std::atomic<socket_factory*> socket_factory::_instance(nullptr);
critical_section socket_factory::_init_cs;

static void freeaddrinfo_wrap(void* const p) noexcept
{
	::freeaddrinfo( static_cast<::addrinfo*>(p) );
}

static s_socket creatate_tcp_socket(std::error_code& ec, ::addrinfo *addr, uint16_t port) noexcept
{
	endpoint ep( std::shared_ptr<::addrinfo>(addr, freeaddrinfo_wrap ) );
	ep.set_port( port );
	return s_socket( nobadalloc<inet_socket>::construct(ec, std::move(ep), transport::tcp ) );
}

socket_factory::~socket_factory() noexcept
{}

void socket_factory::do_release() noexcept
{
	socket_factory *iosrv = _instance.load(std::memory_order_acquire);
	if(nullptr != iosrv)
		delete iosrv;
	_instance.store(nullptr, std::memory_order_release);
}

const socket_factory* socket_factory::instance(std::error_code& ec) noexcept
{
	socket_factory *ret = _instance.load(std::memory_order_relaxed);
	if(nullptr == ret) {
		lock_guard lock(_init_cs);
		ret = _instance.load(std::memory_order_acquire);
		if(nullptr == ret) {
			std::atexit(&socket_factory::do_release);
			ret = nobadalloc<socket_factory>::construct(ec);
			_instance.store(ret, std::memory_order_release);
		}
	}
	return ret;
}

s_socket socket_factory::client_tcp_socket(std::error_code& ec, const char* host, uint16_t port) const noexcept
{
	::addrinfo *addr = nullptr;
	int err = ::getaddrinfo(host, nullptr, nullptr, &addr);
	if(0 != err) {
		ec = std::make_error_code(std::errc::no_such_device_or_address);
		if(nullptr != addr)
			::freeaddrinfo(addr);
		return s_socket();
	}
	switch(addr[0].ai_family) {
	case AF_INET:
	case AF_INET6:
		return creatate_tcp_socket(ec, addr, port);
	default:
		if(nullptr != addr)
			::freeaddrinfo(addr);
		ec = std::make_error_code(std::errc::operation_not_permitted);
	}
	return s_socket();
}

s_socket socket_factory::client_udp_socket(std::error_code& ec, const char* host, uint16_t port) const noexcept
{
	return s_socket();
}

} // namespace net

} // namespace io
