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

namespace io {

namespace net {

namespace secure {

class encryption {
public:

	static const encryption* get(std::error_code& ec) noexcept
	{
		encryption* ret = _instance.load(std::memory_order_relaxed);
		if(nullptr == ret) {
			lock_guard lock(_init_cs);
			ret = _instance.load(std::memory_order_acquire);
			if(nullptr == ret) {
				::tls_init();
				std::shared_ptr<tls_config> cnf = configure();
				if(!cnf) {
					ec = std::make_error_code( std::errc::inappropriate_io_control_operation );
					_instance.store(nullptr, std::memory_order_release);
					return nullptr;
				}
				void *p = memory_traits::malloc( sizeof(encryption) );
				if(nullptr == p) {
					_instance.store(nullptr, std::memory_order_release);
					ec = std::make_error_code(std::errc::not_enough_memory);
					return nullptr;
				}
				ret = new (p) encryption( std::move(cnf) );
			}
		}
		return ret;
	}

	std::shared_ptr<::tls> new_context(bool allow_insecure) const noexcept
	{
		::tls* ret = ::tls_client();
		if(nullptr != ret) {
			if( allow_insecure ) {
				::tls_config_insecure_noverifycert(config_.get());
				::tls_config_insecure_noverifyname(config_.get());
			} else {
				::tls_config_verify(config_.get());
			}
			::tls_configure(ret, config_.get() );
			return std::shared_ptr<::tls>(ret, &encryption::do_release_context);
		}
		return std::shared_ptr<::tls>();
	}

private:
	static void do_release_context(void * const px) noexcept
	{
		::tls_free( static_cast<::tls*>(px) );
	}
	static void do_release_config(void * const px) noexcept
	{
		::tls_config_free(static_cast<::tls_config*>(px) );
	}
	static std::shared_ptr<::tls_config> configure() noexcept
	{
		::tls_config *cnf = ::tls_config_new();
		if(nullptr == cnf)
			return std::shared_ptr<tls_config>();
		return std::shared_ptr<::tls_config>(cnf, &encryption::do_release_config);
	}
	encryption(std::shared_ptr<tls_config>&& config) noexcept:
		config_(config)
	{
		::tls_config_set_protocols(config_.get(), TLS_PROTOCOL_TLSv1);
		::tls_config_set_ca_path(config_.get(), "c:\\Program Files (x86)\\Git\\ssl\\cert");
	}
private:
	std::shared_ptr<::tls_config> config_;
	static critical_section _init_cs;
	static std::atomic<encryption*> _instance;
};

critical_section encryption::_init_cs;
std::atomic<encryption*> encryption::_instance(nullptr);

// channel
#define __check_ec( __ec ) \
if( (__ec) ) return s_read_write_channel();

s_read_write_channel channel::tcp_tls_channel(std::error_code& ec, const char* host, uint16_t port, bool allow_insecure) noexcept
{
	const socket_factory* sf = socket_factory::instance(ec);
	__check_ec(ec);
	s_socket sock = sf->client_tcp_socket(ec, host, port);
	__check_ec(ec);
	s_read_write_channel pure = sock->connect(ec);
	__check_ec(ec);
	const encryption* enc = encryption::get(ec);
	__check_ec( ec );
	std::shared_ptr<::tls> cntx = enc->new_context(allow_insecure);
	if(!cntx) {
		ec = std::make_error_code( std::errc::inappropriate_io_control_operation );
		return s_read_write_channel();
	}
	net::synch_socket_channel *sch = reinterpret_cast<net::synch_socket_channel*>( pure.get() );
	int err = ::tls_connect_socket(cntx.get(), static_cast<int>(sch->socket_), host );
	if(err < 0) {
		std::printf( ::tls_error(cntx.get()) );
		ec = std::make_error_code( std::errc::inappropriate_io_control_operation );
		return s_read_write_channel();
	}
	channel *ret = nobadalloc<channel>::construct(ec, std::move(cntx), std::move(pure) );
	return nullptr != ret ? s_read_write_channel(ret) : s_read_write_channel();
}

#undef __check_ec

channel::channel(std::shared_ptr<::tls>&& cntx, const s_read_write_channel& socket) noexcept:
	read_write_channel(),
	tls_context_( std::forward<std::shared_ptr<::tls> >(cntx) ),
	pure_(socket)
{
}

channel::~channel() noexcept
{
	::tls_close( tls_context_.get() );
}

std::size_t channel::read(std::error_code& ec,uint8_t* const buff, std::size_t bytes) const noexcept
{
	ssize_t ret = ::tls_read( tls_context_.get(), buff, bytes );
	if(ret < 0) {
		std::printf( ::tls_error(tls_context_.get()) );
		ec = std::make_error_code( std::errc::broken_pipe );
		return 0;
	}
	return static_cast<std::size_t>( ret );
	//return pure_->read(ec, buff, bytes);
}

std::size_t channel::write(std::error_code& ec, const uint8_t* buff,std::size_t size) const noexcept
{
	ssize_t ret = ::tls_write(tls_context_.get(), buff, size);
	if(ret < 0) {
		std::printf( ::tls_error(tls_context_.get()) );
		ec = std::make_error_code( std::errc::broken_pipe );
		return 0;
	}
	return static_cast<std::size_t>( ret );
	//return pure_->write(ec, buff, size);
}

} // namespace secure

} // namespace net

} // namespace io
