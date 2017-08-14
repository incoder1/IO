#ifndef __SOCKETS_HPP_INCLUDED__
#define __SOCKETS_HPP_INCLUDED__

#include <config.hpp>

#ifndef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <ws2tcpip.h>

#include <atomic>
#include <channels.hpp>
#include "criticalsection.hpp"

namespace io {

namespace net {

enum class socket_type {
	TCP,
	UDP,
	ICMP
};

class IO_PUBLIC_SYMBOL socket:public virtual object {
protected:
	socket() noexcept;
public:
	virtual socket_type type() const noexcept = 0;
	virtual s_read_write_channel connect(std::error_code& ec) const noexcept = 0;
};

DECLARE_IPTR(socket);

class IO_PUBLIC_SYMBOL socket_factory
{
	socket_factory(const socket_factory&) = delete;
	socket_factory& operator=(const socket_factory&) = delete;
private:
	friend class nobadalloc<socket_factory>;
	static void do_release() noexcept;
	constexpr socket_factory() noexcept
	{}
public:
	~socket_factory() noexcept;
	static const socket_factory* instance(std::error_code& ec) noexcept;
	s_socket client_socket(std::error_code& ec, const char* host, uint16_t port) const noexcept;
private:
	//static socket_address_v4 by_address(std::error_code& ec,const char* ip_address, uint16_t port) noexcept;
private:
	static std::atomic<socket_factory*> _instance;
	static critical_section _init_cs;
};


} //namespace net

} // namespace io


#endif // __SOCKETS_HPP_INCLUDED__
