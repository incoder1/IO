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
#ifndef __IO_TLS_CREDENTIALS_HPP_INCLUDED__
#define __IO_TLS_CREDENTIALS_HPP_INCLUDED__

#include <io/config/libio_config.hpp>

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <gnutls/gnutls.h>
#include <gnutls/x509.h>

namespace io {

namespace net {

namespace tls {

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


} // namespace tls

} //namespace net

} // namespace io

#endif // __IO_TLS_CREDENTIALS_HPP_INCLUDED__
