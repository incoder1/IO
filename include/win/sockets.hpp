#ifndef __SOCKETS_HPP_INCLUDED__
#define __SOCKETS_HPP_INCLUDED__

#include <config.hpp>

#ifndef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <channels.hpp>

namespace io {

namespace net {

namespace inet {

struct socket_address_v4
{
	uint8_t address[4];
	uint16_t port;
};

struct socket_address_v6 {
	uint8_t address[6];
	uint16_t port;
};

namespace tcp {

class client_connection
{
public:
	client_connection(socket_address_v4 addr) noexcept;
	client_connection(socket_address_v6 addr) noexcept;
	s_read_write_channel open(std::error_code& ec) noexcept;
};

} // namespace tcp

// DECLARE_IPTR(socket);


} // namespace inet

} //namespace net

} // namespace io


#endif // __SOCKETS_HPP_INCLUDED__
