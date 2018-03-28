#include "stdafx.hpp"
#include "tls/openssl_secure_channel.hpp"

namespace io {

namespace secure {


static void init_openssl(std::error_code& ec) noexcept {
	#if OPENSSL_VERSION_NUMBER < 0x10100000L
		if( 0 != ::SSL_library_init() ) {
			ec = std::make_error_code( std::errc::broken_pipe );
		}
	#else
		if( OPENSSL_init_ssl(OPENSSL_INIT_LOAD_SSL_STRINGS, NULL) ) {
			ec = std::make_error_code( std::errc::broken_pipe );
		}
	#endif
}

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
			init_openssl(ec);
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
