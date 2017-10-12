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

/**
    Transport Layer Security channels implementations on top of GNU TLS library http://www.gnutls.org/

    WARN! According to the LGPL V2.1 you must link GNU TLS dynamically.
    Commerce usage in non open source software is allowed.
    See: https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html

    Boost Software License can be used with LGPL

    See: https://www.gnu.org/licenses/license-list.en.html#GPLCompatibleLicenses

*/
#ifndef __IO_SECURE_CHANNEL_HPP_INCLUDED__
#define __IO_SECURE_CHANNEL_HPP_INCLUDED__

#include "config.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <atomic>
#include <memory>

// GNU TLS reference
#include <gnutls/gnutls.h>
#include <gnutls/x509.h>

#include <channels.hpp>
#include <network.hpp>

namespace io {

namespace net {

namespace secure {

class IO_PUBLIC_SYMBOL service {
public:
    static const service* instance(std::error_code& ec) noexcept;
    ~service() noexcept;

    s_read_write_channel new_tls_connection(std::error_code& ec, const s_read_write_channel& socket) const noexcept;
private:
    service() noexcept;
    static void destroy_gnu_tls_atexit() noexcept;
private:
    static std::atomic<service*> _instance;
    static critical_section _mtx;
    gnutls_certificate_credentials_t xcred_;
};

/*
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
	s_read_write_channel pure_;
};
*/

} // namespace secure

} // namespace net

} // namespace io

#endif // __IO_SECURE_CHANNEL_HPP_INCLUDED__
