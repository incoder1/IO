#ifndef __IO_OPENSSL_SECURE_CHANNEL_HPP_INCLUDED__
#define __IO_OPENSSL_SECURE_CHANNEL_HPP_INCLUDED__

#include "config.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <atomic>
#include <channels.hpp>
#include <threading.hpp>

#include <openssl/bio.h>
#include <openssl/conf.h>
#include <openssl/engine.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/x509_vfy.h>

namespace io {

namespace secure {

class client_context
{
	client_context(const client_context&) = delete;
	client_context& operator=(const client_context&) = delete;
public:
	constexpr client_context() noexcept:
		ssl_ctx_(nullptr)
	{}
	client_context(client_context&& c) noexcept:
		ssl_ctx_(c.ssl_ctx_)
	{
		c.ssl_ctx_ = nullptr;
	}
	client_context& operator=(client_context&& rhs) noexcept {
		client_context( std::forward<client_context>(rhs) ).swap( *this );
		return *this;
	}
	~client_context() noexcept;
	inline void swap(client_context& other) noexcept {
		std::swap(ssl_ctx_, other.ssl_ctx_);
	}

	inline ::SSL_CTX* get() const noexcept {
		return ssl_ctx_;
	}

	static client_context create(std::error_code& ec) noexcept;
private:
	constexpr client_context(::SSL_CTX* const sslctx) noexcept:
		ssl_ctx_(sslctx)
	{}
private:
	::SSL_CTX* ssl_ctx_;
};

class session {
	session(const session&) = delete;
	session& operator=(session&) = delete;
public:

	static session open(std::error_code& ec, ::SSL_CTX* const cntx) noexcept;

	constexpr session() noexcept:
		ossl_(nullptr),
		rbuf_(nullptr),
		wbuf_(nullptr)
	{}

	session(session&& c) noexcept;

	session& operator=(session&& rhs) noexcept
	{
		session( std::forward<session>(rhs) ).swap( *this );
		return *this;
	}

	bool hand_stake(std::error_code& ec,const char* host, const s_read_write_channel& raw) noexcept;

	std::size_t read(std::error_code& ec, uint8_t * const to, std::size_t bytes) const noexcept;

	std::size_t write(std::error_code& ec, const uint8_t *what, std::size_t length) const noexcept;

	~session() noexcept;

	inline void swap(session& other) noexcept {
        std::swap(ossl_, other.ossl_);
	}

private:
	session(::SSL* const ossl, ::BIO* const rbuf,::BIO* const wbub) noexcept;
private:
	::SSL *ossl_;
	::BIO *rbuf_;
	::BIO *wbuf_;
};

class IO_PUBLIC_SYMBOL tls_channel final: public read_write_channel
{
public:
	virtual ~tls_channel() noexcept override;
	virtual std::size_t read(std::error_code& ec,uint8_t* const buff, std::size_t bytes) const noexcept override;
	virtual std::size_t write(std::error_code& ec, const uint8_t* buff,std::size_t size) const noexcept override;
private:
    friend class nobadalloc<tls_channel>;
    tls_channel(session&& tlssession) noexcept;
private:
    session session_;
};

class IO_PUBLIC_SYMBOL service {
public:
    static const service* instance(std::error_code& ec) noexcept;
    ~service() noexcept;
    s_read_write_channel new_client_connection(std::error_code& ec, s_read_write_channel&& socket) const noexcept;
private:
    service(client_context&& cl_ctx) noexcept;
    static void destroy_openssl_atexit() noexcept;
private:
    static std::atomic<service*> _instance;
    static critical_section _mtx;
    client_context client_context_;
};


} // namespace secure

} // namespace io

#endif // __IO_OPENSSL_SECURE_CHANNEL_HPP_INCLUDED__
