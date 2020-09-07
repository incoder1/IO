/*
 *
 * Copyright (c) 2016-2020
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef __IO_SECURE_CHANNEL_HPP_INCLUDED__
#define __IO_SECURE_CHANNEL_HPP_INCLUDED__

#include "config.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#if defined(IO_TLS_PROVIDER_GNUTLS)
#	include "tls/gnutls_secure_channel.hpp"
#else
#	error Secure transport layer is not provided
#endif // IO_TLS_PROVIDER_GNUTSL

#endif // __IO_SECURE_CHANNEL_HPP_INCLUDED__
