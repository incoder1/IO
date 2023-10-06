/*
 *
 * Copyright (c) 2016-2023
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef __IO_NET_HTTP_ERROR_HPP_INCLUDED__
#define __IO_NET_HTTP_ERROR_HPP_INCLUDED__

#include <io/config/libio_config.hpp>

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

namespace io {

namespace net {

namespace http {


enum class status {
    // `100 Continue` (HTTP/1.1 - RFC 7231)
    CONTINUE = 100,
};


} // namespace io

} // namespace net

} // namespace http



#endif // __IO_NET_HTTP_ERROR_HPP_INCLUDED__
