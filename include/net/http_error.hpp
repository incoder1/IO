#ifndef __IO_HTTP_ERROR_HPP_INCLUDED__
#define __IO_HTTP_ERROR_HPP_INCLUDED__

#include <config.hpp>

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



#endif // __IO_HTTP_ERROR_HPP_INCLUDED__
