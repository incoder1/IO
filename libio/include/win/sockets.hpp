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
#ifndef __SOCKETS_HPP_INCLUDED__
#define __SOCKETS_HPP_INCLUDED__

#include "config.hpp"

#ifndef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#ifndef SECURITY_WIN32
#	define SECURITY_WIN32
#endif // SECURITY_WIN32


#include <atomic>

#include "criticalsection.hpp"
#include "conststring.hpp"
#include "wsaerror.hpp"

#ifndef BTHPROTO_RFCOMM
#	define BTHPROTO_RFCOMM 3
#endif // BTHPROTO_RFCOMM

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

/// Network connection end point, i.e. host, port IP version (4 or 6)
class IO_PUBLIC_SYMBOL endpoint {
private:
	static void do_nothing(void const *) noexcept
	{}
	explicit endpoint(::addrinfo* const info) noexcept:
		addr_info_(info, &endpoint::do_nothing )
	{}
public:
	constexpr endpoint() noexcept:
		addr_info_()
	{}
	bool has_next() const noexcept {
		return addr_info_ && nullptr != addr_info_->ai_next;
	}
	endpoint next() const noexcept {
		return has_next() ? endpoint( addr_info_->ai_next ) : endpoint();
	}
	explicit endpoint(const std::shared_ptr<::addrinfo>& info) noexcept;
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

///  entry point to operating system network
class IO_PUBLIC_SYMBOL socket_factory {
	socket_factory(const socket_factory&) = delete;
	socket_factory& operator=(const socket_factory&) = delete;
private:
	friend class nobadalloc<socket_factory>;
	static void do_release() noexcept;
	constexpr socket_factory() noexcept
	{}
	std::shared_ptr<::addrinfo> get_host_by_name(std::error_code& ec, const char* host) const noexcept;
	static socket creatate_tcp_socket(std::error_code& ec, ::addrinfo *addr, uint16_t port) noexcept;
public:
	~socket_factory() noexcept;
	static const socket_factory* instance(std::error_code& ec) noexcept;
	/// Creates blocking TCP client socket
	/// \param ec operation error code
	/// \param host host name text representation, i.e. DNS name or IP address (V4 or V6)
	/// \param port network port
	/// \return socket object, empty in case of error
	socket client_tcp_socket(std::error_code& ec, const char* host, uint16_t port) const noexcept;
	/// Creates datagram UDP client socket
	/// \param ec operation error code
	/// \param host host name text representation, i.e. DNS name or IP address (V4 or V6)
	/// \param port network port
	/// \return socket object
	socket client_udp_socket(std::error_code& ec, const char* host, uint16_t port) const noexcept;
private:
	static std::atomic<socket_factory*> _instance;
	static critical_section _init_cs;
};


} //namespace net

} // namespace io


#endif // __SOCKETS_HPP_INCLUDED__
