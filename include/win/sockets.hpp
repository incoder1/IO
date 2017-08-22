#ifndef __SOCKETS_HPP_INCLUDED__
#define __SOCKETS_HPP_INCLUDED__

#include <config.hpp>

#ifndef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <mstcpip.h>
#include <ws2tcpip.h>
#include <rpc.h>
#include <ntdsapi.h>

#include <atomic>
#include <channels.hpp>
#include "criticalsection.hpp"
#include "conststring.hpp"
#include "synch_socket_channel.hpp"

#ifndef BTHPROTO_RFCOMM
#	define BTHPROTO_RFCOMM 3
#endif // BTHPROTO_RFCOMM

namespace io {

namespace net {

enum class transport_prot {
	tcp = IPPROTO_TCP,
	udp = IPPROTO_UDP,
	icmp = IPPROTO_ICMP,
	icmp6 = IPPROTO_ICMPV6,
	rfcomm = BTHPROTO_RFCOMM
};

enum class ip_family {
	ip_v4 = AF_INET,
	ip_v6 = AF_INET6,
	bluetooth = AF_BTH
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
	case ip_family::bluetooth:
		os << "Bluetooth";
		break;
	}
	return os;
}

inline std::ostream& operator<<(std::ostream& os, transport_prot prot)
{
	switch(prot) {
	case transport_prot::tcp:
		os << "TCP";
		break;
	case transport_prot::udp:
		os << "UDP";
		break;
	case transport_prot::icmp:
	case transport_prot::icmp6:
		os << "ICMP";
		break;
	case transport_prot::rfcomm:
		os << "Bluetooth RFCOMM";
		break;
	}
	return os;
}


class IO_PUBLIC_SYMBOL endpoint {
private:
	static void do_nothing(void const *p) noexcept
	{}
	endpoint(::addrinfo* const info) noexcept:
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

class IO_PUBLIC_SYMBOL socket:public virtual object {
protected:
	socket() noexcept;
public:
	virtual endpoint get_endpoint() const noexcept = 0;
	virtual s_read_write_channel connect(std::error_code& ec) const noexcept = 0;
	virtual s_read_write_channel connect_secured(std::error_code& ec) const noexcept = 0;
};

DECLARE_IPTR(socket);

class IO_PUBLIC_SYMBOL socket_factory {
	socket_factory(const socket_factory&) = delete;
	socket_factory& operator=(const socket_factory&) = delete;
private:
	friend class nobadalloc<socket_factory>;
	static void do_release() noexcept;
	constexpr socket_factory() noexcept
	{}
	static s_socket creatate_tcp_socket(std::error_code& ec, ::addrinfo *addr, uint16_t port) noexcept;
public:
	~socket_factory() noexcept;
	static const socket_factory* instance(std::error_code& ec) noexcept;
	s_socket client_socket(std::error_code& ec, const char* host, uint16_t port) const noexcept;
private:
	static std::atomic<socket_factory*> _instance;
	static critical_section _init_cs;
};


} //namespace net

} // namespace io


#endif // __SOCKETS_HPP_INCLUDED__
