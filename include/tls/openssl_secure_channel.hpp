#ifndef __IO_OPENSSL_SECURE_CHANNEL_HPP_INCLUDED__
#define __IO_OPENSSL_SECURE_CHANNEL_HPP_INCLUDED__

#include "config.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <atomic>
#include <channels.hpp>
#include <threading.hpp>

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509_vfy.h>

namespace io {

namespace secure {

class IO_PUBLIC_SYMBOL service {
public:
    static const service* instance(std::error_code& ec) noexcept;
    ~service() noexcept;

    s_read_write_channel new_client_connection(std::error_code& ec, s_read_write_channel&& socket) const noexcept;
private:
    service() noexcept;
    static void destroy_openssl_atexit() noexcept;
private:
    static std::atomic<service*> _instance;
    static critical_section _mtx;
};


} // namespace secure

} // namespace io

#endif // __IO_OPENSSL_SECURE_CHANNEL_HPP_INCLUDED__
