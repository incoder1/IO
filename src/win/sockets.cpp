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


// socket
socket::socket() noexcept:
	object()
{}

// endpoint
endpoint::endpoint(const std::shared_ptr<::addrinfo>& info) noexcept:
	addr_info_(info)
{}


uint16_t endpoint::port() const noexcept
{
	switch( family() ) {
	case ip_family::ip_v4:
		return io_ntohs(
		           reinterpret_cast<::PSOCKADDR_IN>(addr_info_->ai_addr)->sin_port
		       );
	case ip_family::ip_v6:
		return io_ntohs(
		           reinterpret_cast<::PSOCKADDR_IN6>(addr_info_->ai_addr)->sin6_port
		       );
	default:
		return 0;
	}
}

void endpoint::set_port(uint16_t port) noexcept
{
	switch( family() ) {
	case ip_family::ip_v4:
		reinterpret_cast<::PSOCKADDR_IN>(addr_info_->ai_addr)->sin_port = io_htons(port);
		break;
	case ip_family::ip_v6:
		reinterpret_cast<::PSOCKADDR_IN6>(addr_info_->ai_addr)->sin6_port = io_htons(port);
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
	const char* ret = ::InetNtopA(
	                      addr_info_->ai_family,
	                      const_cast<void*>(
	                          static_cast<const void*>(addr_info_->ai_addr)
	                      ),
	                      tmp,
	                      INET6_ADDRSTRLEN);
	return (nullptr != ret) ? const_string(tmp) : const_string();
}


// tcp_socket
class tpc_socket final: public socket {
public:
	tpc_socket(endpoint&& ep) noexcept:
		socket(),
		ep_( std::forward<endpoint>(ep) )
	{}
	virtual endpoint get_endpoint() const noexcept
	{
		return ep_;
	}

	virtual s_read_write_channel connect(std::error_code& ec) const noexcept override
	{
		::SOCKET s = create_socket(ec);
		if(ec)
			return s_read_write_channel();
		const ::addrinfo *ai = static_cast<const ::addrinfo *>(ep_.native());
		if(SOCKET_ERROR == ::connect(s, ai->ai_addr, ai->ai_addrlen) ) {
			ec = std::make_error_code( win::wsa_last_error_to_errc() );
			return s_read_write_channel();
		}
		return s_read_write_channel( nobadalloc<synch_socket_channel>::construct(ec, s ) );
	}
private:
	::SOCKET create_socket(std::error_code& ec) const noexcept
	{
		::SOCKET ret = ::WSASocketA( static_cast<int>(
		                                 ep_.family()
		                             ), SOCK_STREAM, IPPROTO_TCP, nullptr, 0, 0 );
		if(ret == INVALID_SOCKET)
			ec = std::make_error_code( win::wsa_last_error_to_errc() );
		return ret;
	}

private:
	endpoint ep_;
};
// socket_factory
std::atomic<socket_factory*> socket_factory::_instance(nullptr);
critical_section socket_factory::_init_cs;

static void initialize_winsocks2(std::error_code& ec) noexcept
{
	::WSADATA wsadata;
	::DWORD err = ::WSAStartup( MAKEWORD(2,2), &wsadata );
	if (err != 0) {
		ec.assign( ::WSAGetLastError(), std::system_category() );
		::WSACleanup();
	}
}

void socket_factory::do_release() noexcept
{
	socket_factory *iosrv = _instance.load(std::memory_order_acquire);
	if(nullptr != iosrv)
		delete iosrv;
	_instance.store(nullptr, std::memory_order_release);
}

socket_factory::~socket_factory() noexcept
{
	::WSACleanup();
}

const socket_factory* socket_factory::instance(std::error_code& ec) noexcept
{
	socket_factory *ret = _instance.load(std::memory_order_relaxed);
	if(nullptr == ret) {
		lock_guard lock(_init_cs);
		ret = _instance.load(std::memory_order_acquire);
		if(nullptr == ret) {
			initialize_winsocks2(ec);
			if(ec) {
				_instance.store(ret, std::memory_order_release);
				return nullptr;
			}
			std::atexit(&socket_factory::do_release);
			ret = nobadalloc<socket_factory>::construct(ec);
			if(ec) {
				_instance.store(ret, std::memory_order_release);
				return nullptr;
			}
			_instance.store(ret, std::memory_order_release);
		}
	}
	return ret;
}

static void freeaddrinfo_wrap(void* const p) noexcept
{
	::freeaddrinfo( static_cast<::addrinfo*>(p) );
}

s_socket socket_factory::creatate_tcp_socket(std::error_code& ec, ::addrinfo *addr, uint16_t port) noexcept
{
	endpoint ep( std::shared_ptr<::addrinfo>(addr, freeaddrinfo_wrap ) );
	ep.set_port( port );
	return s_socket( nobadalloc<tpc_socket>::construct(ec, std::move(ep) ) );
}

s_socket socket_factory::client_socket(std::error_code& ec, const char* host, uint16_t port) const noexcept
{
	::addrinfo *addr = nullptr;
	int err = ::getaddrinfo(host, nullptr, nullptr, &addr);
	// TODO: should be custom error code
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

} // namespace net

} // namespace io
