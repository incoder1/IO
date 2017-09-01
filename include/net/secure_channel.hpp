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
#ifndef __IO_SECURE_CHANNEL_HPP_INCLUDED__
#define __IO_SECURE_CHANNEL_HPP_INCLUDED__

#include "config.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <atomic>
#include <memory>

// interuction with LibreSSL
#include <tls.h>

#include <channels.hpp>
#include <network.hpp>

namespace io {

namespace net {

namespace secure {

class IO_PUBLIC_SYMBOL channel final: public read_write_channel {
private:
	friend class nobadalloc<channel>;
	channel(std::shared_ptr<::tls>&& cntx, const s_read_write_channel& socket) noexcept;
public:
	static s_read_write_channel tcp_tls_channel(std::error_code& ec, const char* host, uint16_t port, bool allow_insecure) noexcept;
	virtual ~channel() noexcept override;
	virtual std::size_t read(std::error_code& ec,uint8_t* const buff, std::size_t bytes) const noexcept override;
	virtual std::size_t write(std::error_code& ec, const uint8_t* buff,std::size_t size) const noexcept override;
public:
	std::shared_ptr<::tls> tls_context_;
	s_read_write_channel pure_;
};

} // namespace secure

} // namespace net

} // namespace io

#endif // __IO_SECURE_CHANNEL_HPP_INCLUDED__
