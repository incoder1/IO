/*
 *
 * Copyright (c) 2016-2023
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef __IO_CORE_STATEFUL_HPP_INCLUDED__
#define __IO_CORE_STATEFUL_HPP_INCLUDED__

#include <io/config/libio_config.hpp>

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include "channels.hpp"

namespace io {

class pump;
DECLARE_IPTR(pump);

class IO_PUBLIC_SYMBOL pump: public object {
	pump(pump&) = delete;
	pump& operator=(pump&) = delete;
protected:
	pump() noexcept;
public:
	virtual std::size_t pull(std::error_code& ec, uint8_t* const to, std::size_t bytes) noexcept = 0;
	virtual bool sync(std::error_code& ec) noexcept;
};

class IO_PUBLIC_SYMBOL channel_pump: public pump {
protected:
	explicit channel_pump(s_read_channel&& src) noexcept;
public:
	static s_pump create(std::error_code& ec, s_read_channel&& src) noexcept;
	virtual std::size_t pull(std::error_code& ec, uint8_t* const to, std::size_t bytes) noexcept override;
private:
	s_read_channel src_;
};

class IO_PUBLIC_SYMBOL buffered_channel_pump: public channel_pump {
protected:
	buffered_channel_pump(s_read_channel&& src,byte_buffer&& buff) noexcept;
public:
	static s_pump create(std::error_code& ec, s_read_channel&& src, std::size_t buffer_size) noexcept;
	virtual std::size_t pull(std::error_code& ec, uint8_t* const to, std::size_t bytes) noexcept override;
	virtual bool sync(std::error_code& ec) noexcept override;
protected:
	byte_buffer read_buff_;
};

class funnel;
DECLARE_IPTR(funnel);

class IO_PUBLIC_SYMBOL funnel: public object {
protected:
	funnel() noexcept;
public:
	virtual std::size_t push(std::error_code& ec, const uint8_t* src, std::size_t bytes) noexcept = 0;
	virtual void flush(std::error_code& ec) noexcept;
};

class IO_PUBLIC_SYMBOL channel_funnel: public funnel
{
protected:
	channel_funnel(s_write_channel&& dst) noexcept;
public:
	static s_funnel create(std::error_code& ec, s_write_channel&& dst) noexcept;
	virtual std::size_t push(std::error_code& ec, const uint8_t* src, std::size_t bytes) noexcept override;
private:
	s_write_channel dst_;
};

class IO_PUBLIC_SYMBOL buffered_channel_funnel: public channel_funnel {
protected:
	buffered_channel_funnel(s_write_channel&& dst, byte_buffer&& buff) noexcept;
public:
	static s_funnel create(std::error_code& ec, s_write_channel&& dst, std::size_t buffer_size) noexcept;
	virtual std::size_t push(std::error_code& ec, const uint8_t* src, std::size_t bytes) noexcept override;
	virtual void flush(std::error_code& ec) noexcept override;
protected:
	byte_buffer write_buff_;
};

} // namespace io

#endif // __IO_CORE_STATEFUL_HPP_INCLUDED__
