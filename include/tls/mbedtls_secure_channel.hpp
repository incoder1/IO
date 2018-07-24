/*
 *
 * Copyright (c) 2018
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef __IO_MBEDTLS_SECURE_CHANNEL_HPP_INCLUDED__
#define __IO_MBEDTLS_SECURE_CHANNEL_HPP_INCLUDED__

#include "config.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <mbedtls/ssl.h>

// io libs
#include <channels.hpp>
#include <threading.hpp>

namespace io {

namespace secure {


class IO_PUBLIC_SYMBOL service {
	service(const service&) = delete;
	service& operator=(const service&) = delete;
public:
    static const service* instance(std::error_code& ec) noexcept;
    ~service() noexcept;

    s_read_write_channel new_client_connection(std::error_code& ec, s_read_write_channel&& socket) const noexcept;
private:
    static void destroy_engine_atexit() noexcept;
    service(::mbedtls_ssl_context&& context, ::mbedtls_ssl_config&& config) noexcept;
private:
    static std::atomic<service*> _instance;
    static critical_section _mtx;
    ::mbedtls_ssl_context contex_;
    ::mbedtls_ssl_config config_;
};

} // namespace secure

} // namespace io

#endif // __IO_MBEDTLS_SECURE_CHANNEL_HPP_INCLUDED__
