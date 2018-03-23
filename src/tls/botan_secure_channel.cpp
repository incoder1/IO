/*
 *
 * Copyright (c) 2018
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#include "stdafx.hpp"
#include "tls/botan_secure_channel.hpp"

namespace io {

namespace net {

namespace secure {

void service::destroy_botan_atexit() noexcept
{
    service *srv = _instance.load(std::memory_order_relaxed);
    if(nullptr != srv) {
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
            if(!ec) {
                std::atexit( &service::destroy_botan_atexit );
                ret = new (std::nothrow) service( );
            }
            _instance.store(ret, std::memory_order_release);
        }
    }
    return ret;
}


} // namespace secure

} // namespace net

} // namespace io
