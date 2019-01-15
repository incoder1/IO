#ifndef __IO_SECURE_CHANNEL_HPP_INCLUDED__
#define __IO_SECURE_CHANNEL_HPP_INCLUDED__

#include "config.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#if defined(IO_TLS_PROVIDER_GNUTLS)
#	include "tls/gnutls_secure_channel.hpp"
#elif defined(IO_TLS_PROVIDER_MBEDTLS)
#	include "tls/mbedtls_secure_channel.hpp"
#else
#	error Secure transport layer is not provided
#endif // IO_TLS_PROVIDER_GNUTSL

#endif // __IO_SECURE_CHANNEL_HPP_INCLUDED__
