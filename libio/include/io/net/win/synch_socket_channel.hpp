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
#ifndef __IO_WIN_SYNCH_SOCKET_CHANNEL_HPP_INCLUDED__
#define __IO_WIN_SYNCH_SOCKET_CHANNEL_HPP_INCLUDED__

#include <io/config/libio_config.hpp>

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#ifndef SECURITY_WIN32
#	define SECURITY_WIN32
#endif // SECURITY_WIN32

#include <io/core/channels.hpp>
#include "wsaerror.hpp"

namespace io {

namespace net {


class synch_socket_channel :public read_write_channel {
public:
	explicit synch_socket_channel(::SOCKET socket) noexcept;
	virtual ~synch_socket_channel() noexcept;
	virtual std::size_t read(std::error_code& ec,uint8_t* const buff, std::size_t bytes) const noexcept override;
	virtual std::size_t write(std::error_code& ec, const uint8_t* buff,std::size_t size) const noexcept override;
public:
	::SOCKET socket_;
};

} // namespace net

} // namespace io



#endif // __IO_WIN_SYNCH_SOCKET_CHANNEL_HPP_INCLUDED__
