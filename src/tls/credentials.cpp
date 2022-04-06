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
#include "stdafx.hpp"
#include "credentials.hpp"

namespace io {

namespace net {

namespace tls {

// credentials
credentials credentials::system_trust_creds(std::error_code& ec) noexcept
{
    credentials ret;
    if( GNUTLS_E_SUCCESS != ::gnutls_certificate_allocate_credentials( &ret.creds_ ) )
        ec = std::make_error_code( std::errc::protocol_error );
    else if( GNUTLS_E_UNIMPLEMENTED_FEATURE == ::gnutls_certificate_set_x509_system_trust( ret.creds_ ) )
        ec = std::make_error_code( std::errc::protocol_error );
    return ret;
}

} // namespace tls

} //namespace net

} // namespace io

