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
#include "stdafx.hpp"
#include "secure_channel.hpp"
#include "threading.hpp"

#ifdef _WIN32
#   include <Wincrypt.h>
#endif // _WIN32


namespace io {

namespace net {

namespace secure {

std::atomic<service*> service::_instance(nullptr);
critical_section service::_mtx;

void service::destroy_gnu_tls_atexit() noexcept
{
    service *srv = _instance.load(std::memory_order_relaxed);
    if(nullptr != srv) {
        ::gnutls_global_deinit();
        delete srv;
    }
}

const service* service::instance(std::error_code& ec) noexcept
{
    service *ret = _instance.load(std::memory_order_consume);
    if(nullptr == ret) {
        lock_guard lock(_mtx);
        ret = _instance.load(std::memory_order_consume);
        if(nullptr == ret) {
            if( GNUTLS_E_SUCCESS != ::gnutls_global_init() ) {
                ec = std::make_error_code(std::errc::io_error);
                return nullptr;
            }
            std::atexit( &service::destroy_gnu_tls_atexit );
            ret = new (std::nothrow) service();
            if(nullptr == ret)
                ec = std::make_error_code(std::errc::not_enough_memory);
            _instance.store(ret, std::memory_order_release);
        }
    }
    return ret;
}

service::service() noexcept
{
    ::gnutls_certificate_allocate_credentials( &xcred_ );
    ::gnutls_certificate_set_x509_system_trust( xcred_ );

/*
#ifdef _WIN32
    ::HCERTSTORE store = ::CertOpenSystemStoreW( static_cast<HCRYPTPROV_LEGACY>(NULL), L"ROOT");
    ::PCCERT_CONTEXT context = nullptr;
    std::error_code ec;
    byte_buffer buff = byte_buffer::allocate(ec, memory_traits::page_size() );
     for(;;) {
        context = ::CertEnumCertificatesInStore(store, context);
        if(nullptr == context)
            break;
        buff.put();
     }
     gnutls_datum_t dataum {

     };
    ::gnutls_certificate_set_x509_trust_mem(xcred_,  , GNUTLS_X509_FMT_DER);
#endif // _WIN32
*/

}

service::~service() noexcept
{
    ::gnutls_certificate_free_credentials(xcred_);
}

s_read_write_channel service::new_tls_connection(std::error_code& ec, const s_read_write_channel& socket) const noexcept
{
    ::gnutls_session_t session;
    ::gnutls_init(&session, GNUTLS_CLIENT);
    ::gnutls_set_default_priority(session);

    const synch_socket_channel* s = reinterpret_cast<synch_socket_channel*>( socket.get() );

    ::gnutls_transport_set_int( session, s->socket_ );
    ::gnutls_handshake_set_timeout(session, GNUTLS_DEFAULT_HANDSHAKE_TIMEOUT);
    int err;
    do {
        err = gnutls_handshake(session);
    } while (err < 0 && ::gnutls_error_is_fatal(err) == 0);

}


} // namespace secure

} // namespace net

} // namespace io
