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
#include "mbedtls_secure_channel.hpp"

namespace io {

namespace secure {

// service
std::atomic<service*> service::_instance(nullptr);
critical_section service::_mtx;

static bool init_mbedtls(::mbedtls_ssl_context& contex,::mbedtls_ssl_config& config)
{
	::mbedtls_ssl_init(&contex);
	::mbedtls_ssl_config_init(&config);
	if( 0 != mbedtls_ssl_config_defaults(
				&config,
				MBEDTLS_SSL_IS_CLIENT,
                MBEDTLS_SSL_TRANSPORT_STREAM,
				MBEDTLS_SSL_PRESET_DEFAULT ) )
		return false;
	else
        return 0 == ::mbedtls_ssl_setup(&contex,&config);
}

const service* service::instance(std::error_code& ec) noexcept
{
	service *ret = _instance.load(std::memory_order_consume);
	if(nullptr == ret) {
		lock_guard lock(_mtx);
		ret = _instance.load(std::memory_order_consume);
		if(nullptr == ret) {
			::mbedtls_ssl_context contex;
    		::mbedtls_ssl_config config;
			if( init_mbedtls(contex,config) ) {
				std::atexit( &service::destroy_engine_atexit );
				ret = new (std::nothrow) service( std::move(contex), std::move(config) );
				if(nullptr == ret)
					ec = std::make_error_code(std::errc::not_enough_memory);
			} else
				ec = std::make_error_code(std::errc::not_enough_memory);
			_instance.store(ret, std::memory_order_release);
		}
	}
	return ret;
}

void service::destroy_engine_atexit() noexcept
{
	service *srv = _instance.load(std::memory_order_relaxed);
	delete srv;
}

service::service(::mbedtls_ssl_context&& context, ::mbedtls_ssl_config&& config) noexcept:
	contex_( std::forward<::mbedtls_ssl_context>(context) ),
	config_( std::forward<::mbedtls_ssl_config>(config) )
{}

service::~service() noexcept
{
    ::mbedtls_ssl_free(&contex_);
    ::mbedtls_ssl_config_free(&config_);
}

s_read_write_channel service::new_client_connection(std::error_code& ec, s_read_write_channel&& socket) const noexcept
{
	ec = std::make_error_code(std::errc::function_not_supported);
	return s_read_write_channel();
}


} // namespace secure

} // namespace io
