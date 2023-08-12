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
#ifndef __POSIX_SYNCH_SOCKET_CHANNEL_HPP__INCLUDED__
#define __POSIX_SYNCH_SOCKET_CHANNEL_HPP__INCLUDED__

#include "config.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>

#include "channels.hpp"

namespace io {

namespace net {

class synch_socket_channel final:public read_write_channel {
private:
	static constexpr int SOCKET_ERROR = -1;
	friend class nobadalloc<synch_socket_channel>;
public:
	explicit synch_socket_channel(int socket) noexcept;
	virtual ~synch_socket_channel() noexcept;
	virtual std::size_t read(std::error_code& ec,uint8_t* const buff, std::size_t bytes) const noexcept override;
	virtual std::size_t write(std::error_code& ec, const uint8_t* buff,std::size_t size) const noexcept override;
public:
	os_descriptor_t socket_;
};


} // namespace net

} // namespace io

#endif // __POSIX_SYNCH_SOCKET_CHANNEL_HPP__INCLUDED__
