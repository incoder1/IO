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
#ifndef __IO_POSIX_SOCKETS_HPP_INCLUDED__
#define __IO_POSIX_SOCKETS_HPP_INCLUDED__

#include <config.hpp>

#ifndef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <atomic>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "criticalsection.hpp"
#include "conststring.hpp"

namespace io {

namespace net {

enum class transport {
	tcp = IPPROTO_TCP,
	udp = IPPROTO_UDP,
	icmp = IPPROTO_ICMP,
	icmp6 = IPPROTO_ICMPV6
};

enum class ip_family {
	ip_v4 = AF_INET,
	ip_v6 = AF_INET6,
};

inline std::ostream& operator<<(std::ostream& os, ip_family ipf)
{
	switch(ipf) {
	case ip_family::ip_v4:
		os << "TCP/IP version 4";
		break;
	case ip_family::ip_v6:
		os << "TCP/IP version 6";
		break;
	}
	return os;
}

inline std::ostream& operator<<(std::ostream& os, transport prot)
{
	switch(prot) {
	case transport::tcp:
		os << "TCP";
		break;
	case transport::udp:
		os << "UDP";
		break;
	case transport::icmp:
	case transport::icmp6:
		os << "ICMP";
		break;
	}
	return os;
}

class IO_PUBLIC_SYMBOL endpoint {
private:
	static void do_nothing(void const *p) noexcept
	{}
	endpoint(::addrinfo* info) noexcept:
		addr_info_(info, &endpoint::do_nothing )
	{}
public:
	endpoint() noexcept:
		addr_info_()
	{}
	bool has_next() const noexcept {
		return addr_info_ && nullptr != addr_info_->ai_next;
	}
	endpoint next() const noexcept {
		return has_next() ? endpoint( addr_info_->ai_next ) : endpoint();
	}
	endpoint(std::shared_ptr<::addrinfo>&& info) noexcept;
	endpoint(const std::shared_ptr<::addrinfo>& info) noexcept;
	const_string ip_address() const noexcept;
	uint16_t port() const noexcept;
	void set_port(uint16_t port) noexcept;
	ip_family family() const noexcept;
	const void* native() const noexcept;
private:
	std::shared_ptr<::addrinfo> addr_info_;
};

/// Network socket interface
class IO_PUBLIC_SYMBOL socket {
public:
    socket(endpoint&& ep, transport t_prot) noexcept:
        transport_(t_prot),
        ep_( std::forward<endpoint>(ep) )
	{}

	socket() noexcept:
		socket( endpoint(), transport::tcp)
	{}

	/// Returns this socket endpoint
	/// \return socket endpoint
    endpoint get_endpoint() const noexcept {
        return ep_;
    }

	/// Returns socket transport type, i.e. TCP,UDP or ICMP
	/// \return socket transport
    transport transport_protocol() const noexcept {
        return transport_;
    }

private:
    transport transport_;
    endpoint ep_;
};

class IO_PUBLIC_SYMBOL socket_factory {
	socket_factory(const socket_factory&) = delete;
	socket_factory& operator=(const socket_factory&) = delete;
private:
	friend class nobadalloc<socket_factory>;
	static void do_release() noexcept;
	constexpr socket_factory() noexcept
	{}
	std::shared_ptr<::addrinfo> get_host_by_name(std::error_code& ec, const char* host) const noexcept;
public:
	~socket_factory() noexcept;
	static const socket_factory* instance(std::error_code& ec) noexcept;
	socket client_tcp_socket(std::error_code& ec, const char* host, uint16_t port) const noexcept;
	socket client_udp_socket(std::error_code& ec, const char* host, uint16_t port) const noexcept;
private:
	static std::atomic<socket_factory*> _instance;
	static critical_section _init_cs;
};

} // namespace net

} // namespace io

#endif // __IO_POSIX_SOCKETS_HPP_INCLUDED__
