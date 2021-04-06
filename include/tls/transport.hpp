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
#ifndef __IO_TLS_TRANSPORT_HPP_INCLUDED__
#define __IO_TLS_TRANSPORT_HPP_INCLUDED__

#include "config.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include "channels.hpp"

namespace io {

namespace net {

namespace tls {

class transport:public io::object
{
	transport(const transport&) = delete;
	transport& operator=(const transport&) = delete;
protected:
	constexpr transport() noexcept:
		object()
	{}
public:
	virtual ssize_t pull(std::error_code& ec, void* dst,std::size_t len) noexcept = 0;

	virtual ssize_t push(std::error_code& ec, const void* src,std::size_t len) noexcept = 0;

	virtual ~transport() noexcept = default;
};

DECLARE_IPTR(transport);

class synch_transport final: public transport {
public:
	synch_transport(io::s_read_write_channel&& raw) noexcept;
	virtual ssize_t pull(std::error_code& ec, void* dst,std::size_t len) noexcept override;
	virtual ssize_t push(std::error_code& ec, const void* src,std::size_t len) noexcept override;
private:
	io::s_read_write_channel raw_;
};

class asynch_transport final: public transport {
public:
	asynch_transport(io::s_asynch_channel&& raw) noexcept;
	virtual ssize_t pull(std::error_code& ec, void* dst,std::size_t len) noexcept override;
	virtual ssize_t push(std::error_code& ec, const void* src,std::size_t len) noexcept override;
private:
	io::s_asynch_channel raw_;
};


} // namespace tls

} //namespace net

} // namespace io

#endif // __IO_TLS_TRANSPORT_HPP_INCLUDED__
