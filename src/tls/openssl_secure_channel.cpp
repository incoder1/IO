#include "stdafx.hpp"
#include "tls/openssl_secure_channel.hpp"

namespace io {

namespace secure {

// service
std::atomic<service*> service::_instance(nullptr);
critical_section service::_mtx;


void service::destroy_openssl_atexit() noexcept
{
    service *srv = _instance.load(std::memory_order_relaxed);
    if(nullptr != srv)
        delete srv;
}

const service* service::instance(std::error_code& ec) noexcept
{
    service *ret = _instance.load(std::memory_order_consume);
    if(nullptr == ret) {
        lock_guard lock(_mtx);
        ret = _instance.load(std::memory_order_consume);
        if(nullptr == ret) {
            if(!ec) {
                std::atexit( &service::destroy_openssl_atexit );
                ret = new (std::nothrow) service();
                if(nullptr == ret)
                    ec = std::make_error_code(std::errc::not_enough_memory);
            }
            _instance.store(ret, std::memory_order_release);
        }
    }
    return ret;
}


} // namespace secure

} // namespace io
