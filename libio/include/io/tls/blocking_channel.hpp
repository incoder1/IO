/*
 *
 * Copyright (c) 2016-2022
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef __IO_TLS_BLOCKING_CHANNEL_HPP_INCLUDED__
#define __IO_TLS_BLOCKING_CHANNEL_HPP_INCLUDED__

#include <io/config/libio_config.hpp>

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <io/core/channels.hpp>

#include "session.hpp"

namespace io {

namespace net {

namespace tls {

class IO_PUBLIC_SYMBOL blocking_channel final: public read_write_channel
{
public:
    blocking_channel(s_session&& session) noexcept;
	virtual ~blocking_channel() noexcept override;
	virtual std::size_t read(std::error_code& ec,uint8_t* const buff, std::size_t bytes) const noexcept override;
	virtual std::size_t write(std::error_code& ec, const uint8_t* buff,std::size_t size) const noexcept override;
private:
    s_session session_;
};

} // namespace tls

} //namespace net

} // namespace io

#endif // __IO_TLS_BLOCKING_CHANNEL_HPP_INCLUDED__
