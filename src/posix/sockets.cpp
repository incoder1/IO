/*
 *
 * Copyright (c) 2016-2021
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#include "stdafx.hpp"
#include "posix/sockets.hpp"

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
	char tmp[INET6_ADDRSTRLEN] = {'\0'};
	const char* ret = ::inet_ntop(
	                      addr_info_->ai_family,
	                      const_cast<void*>(
	                          static_cast<const void*>(addr_info_->ai_addr)
	                      ),
	                      tmp,
	                      INET6_ADDRSTRLEN);
	return nullptr != ret ? const_string(tmp) : const_string() ;
}

// socket_factory
std::atomic<socket_factory*> socket_factory::_instance(nullptr);
critical_section socket_factory::_init_cs;

static void freeaddrinfo_wrap(void* const p) noexcept
{
	::freeaddrinfo( static_cast<::addrinfo*>(p) );
}

static socket creatate_tcp_socket(std::error_code& ec, ::addrinfo *addr, uint16_t port) noexcept
{
	endpoint ep( std::shared_ptr<::addrinfo>(addr, freeaddrinfo_wrap ) );
	ep.set_port( port );
	return socket(std::move(ep), transport::tcp );
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

std::shared_ptr<::addrinfo> socket_factory::get_host_by_name(std::error_code& ec, const char* host) const noexcept
{
    ::addrinfo *ret = nullptr;
    if(0 != ::getaddrinfo(host, nullptr, nullptr, &ret) ) {
        ec = std::make_error_code(std::errc::no_such_device_or_address);
        if(nullptr != ret)
            ::freeaddrinfo(ret);
    }
    return std::shared_ptr<::addrinfo>(ret, freeaddrinfo_wrap );
}

socket socket_factory::client_tcp_socket(std::error_code& ec, const char* host, uint16_t port) const noexcept
{
    endpoint ep( get_host_by_name(ec, host) );
    ep.set_port( port );
    return socket( std::move(ep), transport::tcp );
}

socket socket_factory::client_udp_socket(std::error_code& ec, const char* host, uint16_t port) const noexcept
{
    endpoint ep( get_host_by_name(ec, host) );
    ep.set_port( port );
    return socket( std::move(ep), transport::udp );
}

} // namespace net

} // namespace io
