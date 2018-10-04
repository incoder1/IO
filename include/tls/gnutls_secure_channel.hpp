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
#ifndef __IO_GNUTLS_SECURE_CHANNEL_HPP_INCLUDED__
#define __IO_GNUTLS_SECURE_CHANNEL_HPP_INCLUDED__

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
#include <threading.hpp>

namespace io {

namespace secure {

class credentials
{
    credentials(const credentials& other) = delete;
    credentials& operator=(const credentials& rhs) = delete;
public:

    credentials(credentials&& other) noexcept:
        creds_( other.creds_ )
    {
        other.creds_ = nullptr;
    }

    credentials& operator=(credentials&& rhs) noexcept
    {
        credentials( static_cast<credentials&&>(rhs) ).swap( *this );
        return *this;
    }

    ::gnutls_certificate_credentials_t get() const noexcept
    {
        return creds_;
    }

    inline void swap(credentials& other) noexcept {
        std::swap(creds_, other.creds_ );
    }

    ~credentials() noexcept
    {
        if(nullptr != creds_)
            ::gnutls_certificate_free_credentials(creds_);
    }

    static credentials system_trust_creds(std::error_code& ec) noexcept;
private:
    constexpr credentials() noexcept:
        creds_(nullptr)
    {}
private:
    ::gnutls_certificate_credentials_t creds_;
};



class session {

    session(const session&) = delete;
    session& operator=(const session&) = delete;

public:

    static session client_session(std::error_code &ec, ::gnutls_certificate_credentials_t crd,  s_read_write_channel&& socket) noexcept;

    ~session() noexcept;

    session(session&& other) noexcept;

    session& operator=(session&& rhs) noexcept
    {
        session( std::forward<session>(rhs) ).swap( *this );
        return *this;
    }

    inline void swap(session& other) noexcept
    {
        std::swap(peer_, other.peer_);
        socket_.swap( other.socket_ );
    }

    std::size_t read(std::error_code& ec, uint8_t * const data, std::size_t max_size) const noexcept;

    std::size_t write(std::error_code& ec, const uint8_t *data, std::size_t data_size) const noexcept;

private:


    explicit session(s_read_write_channel&& socket) noexcept;

    static int client_handshake(::gnutls_session_t peer) noexcept;

    static ssize_t push(::gnutls_transport_ptr_t trpt, const void *data, std::size_t size) noexcept;

    static ssize_t pull(::gnutls_transport_ptr_t trpt, void * data, std::size_t max_size) noexcept;

private:
    ::gnutls_session_t peer_;
    s_read_write_channel socket_;
};

class IO_PUBLIC_SYMBOL tls_channel final: public read_write_channel
{
public:
	virtual ~tls_channel() noexcept override;
	virtual std::size_t read(std::error_code& ec,uint8_t* const buff, std::size_t bytes) const noexcept override;
	virtual std::size_t write(std::error_code& ec, const uint8_t* buff,std::size_t size) const noexcept override;
private:
    friend class nobadalloc<tls_channel>;
    tls_channel(session&& session) noexcept;
private:
    session session_;
};


class IO_PUBLIC_SYMBOL service {
	service(const service&) = delete;
	service& operator=(const service&) = delete;
public:
    static const service* instance(std::error_code& ec) noexcept;
    ~service() noexcept;

    s_read_write_channel new_client_connection(std::error_code& ec, s_read_write_channel&& socket) const noexcept;
private:
    static void destroy_gnu_tls_atexit() noexcept;
    service(credentials&& creds) noexcept:
    	creds_( std::forward<credentials>(creds) )
	{}
private:
    static std::atomic<service*> _instance;
    static critical_section _mtx;
    credentials creds_;
};


} // namespace secure

} // namespace io

#endif // __IO_GNUTLS_SECURE_CHANNEL_HPP_INCLUDED__
