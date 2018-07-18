#include "stdafx.hpp"
#include "tls/openssl_secure_channel.hpp"

#ifdef __IO_WINDOWS_BACKEND__

#	ifdef __MINGW64__
#		include <prsht.h>
#	endif // defined

#	include <wincrypt.h>
#	include <cryptuiapi.h>

#endif // __IO_WINDOWS_BACKEND__


namespace io {

namespace secure {


static bool init_openssl() noexcept
{
	::SSL_load_error_strings();
	if( 0 == ::SSL_library_init() )
		return false;
	OpenSSL_add_all_algorithms();
	return true;
}

#ifdef __IO_WINDOWS_BACKEND__
// get X509 certs from SSPI system trust store

static bool load_system_trust_store(::SSL_CTX* const sslctx) noexcept
{
	static constexpr const wchar_t* SYS_NAME = L"ROOT";
	::HCERTSTORE sspi_store = ::CertOpenSystemStoreW( static_cast<HCRYPTPROV_LEGACY>(0), SYS_NAME);
	if(NULL == sspi_store)
		return false;
	::X509_STORE *ossl_store  = ::SSL_CTX_get_cert_store( sslctx );
	::PCCERT_CONTEXT sspi_cert = nullptr;
	::X509 *ossl_cert;
	while( nullptr != (sspi_cert = ::CertEnumCertificatesInStore(sspi_store, sspi_cert) ) ) {
		::BYTE *raw = sspi_cert->pbCertEncoded;
		::DWORD raw_size = sspi_cert->cbCertEncoded;
		ossl_cert = ::d2i_X509(nullptr, const_cast<const unsigned char**>(&raw), raw_size);
		if(nullptr != ossl_cert) {
			::X509_STORE_add_cert(ossl_store, ossl_cert);
			::X509_free(ossl_cert);
		}
	}
	::CertCloseStore(sspi_store, 0);
	return true;
}

#else

static bool load_system_trust_store(::SSL_CTX* const sslctx) noexcept
{
	// TODO: check whether implementation needed
	return true;
}

#endif // __IO_WINDOWS_BACKEND__

// client_context

client_context client_context::create(std::error_code& ec) noexcept
{
	static constexpr const long TLS_FLAGS = SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_TLSv1 | SSL_OP_NO_COMPRESSION;
	::SSL_CTX *ctx = ::SSL_CTX_new( ::SSLv23_method() );

	if(nullptr == ctx) {
		ec = std::make_error_code( std::errc::protocol_not_supported );
		return client_context();
	}
	::SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
	::SSL_CTX_set_options(ctx, TLS_FLAGS);
	if( !load_system_trust_store(ctx) ) {
		ec = std::make_error_code( std::errc::protocol_not_supported );
		return client_context();
	}
	return client_context( ctx );
}


client_context::~client_context() noexcept
{
	if(nullptr != ssl_ctx_)
		::SSL_CTX_free(ssl_ctx_);
}

//session

session session::open(std::error_code& ec,::SSL_CTX* const cntx) noexcept
{
	::SSL* ossl = ::SSL_new(cntx);
	if(nullptr == ossl) {
		ec = std::make_error_code(std::errc::connection_aborted);
		return session();
	}
	::BIO *interal = nullptr, *external = nullptr;
	//std::size_t bio_size = memory_traits::page_size();
	::BIO_new_bio_pair(&interal, 0, &external, 0);
	if(nullptr == interal || nullptr == external ) {
		ec = std::make_error_code(std::errc::not_enough_memory);
		::SSL_free(ossl);
		return session();
	}

	::BIO_set_mem_eof_return(interal, EOF);
	::BIO_set_mem_eof_return(external, EOF);

	::SSL_set_mode(ossl, SSL_MODE_AUTO_RETRY);

	//::BIO_set_conn_hostname(external, ":https");
	::SSL_set_bio(ossl, interal, interal);
	::SSL_set_connect_state(ossl);
	::SSL_connect(ossl);
	::ERR_clear_error();
	return session(ossl, interal, external);
}

session::session(::SSL* const ossl, ::BIO* const internal,::BIO* const external) noexcept:
	ossl_(ossl),
	internal_(internal),
	external_(external)
{
}

session::session(session&& c) noexcept:
	session(c.ossl_,c.internal_,c.external_)
{
	c.ossl_ = nullptr;
	c.internal_ = nullptr;
	c.external_ = nullptr;
}

session::~session() noexcept
{
	if( nullptr != ossl_ ) {
		::SSL_free(ossl_);
		::BIO_free(external_);
	}
}


ssize_t session::get_client_handshake_data(uint8_t* buff, uint16_t size) noexcept
{
	::BIO_ctrl_pending(external_);
	return ::BIO_read(external_, buff , size );
}

ssize_t session::set_server_handshake_data(uint8_t* const buff, uint16_t size) noexcept
{
	ssize_t ret = ::BIO_write( external_, buff , size );
	::BIO_ctrl_pending(external_);
	::SSL_do_handshake(ossl_);
	return ret;
}

int session::get_error(int ret) {
	return ::SSL_get_error(ossl_, ret);
}

bool session::handshake_done() noexcept
{
	return SSL_is_init_finished(ossl_);
}

int session::decrypt(const uint8_t *what, std::size_t size, uint8_t* const to, std::size_t& decrypted) const noexcept
{
	int written = 0, e;
	while(written < size) {
		::BIO_ctrl_pending(external_);
		e = ::BIO_write(external_, (what + written), (size-written) );
		if( e < 0 )
			return -1;
		written += e;
	}
	int ret = ::SSL_read(ossl_, to, written);
	decrypted = (ret > 0) ? static_cast<std::size_t>(ret) : 0;
	return ret;
}

int session::encrypt(const uint8_t *what, std::size_t size, uint8_t* const to, std::size_t& encrypted) const noexcept
{
	int written = 0, e;
	while(written < size) {
		::BIO_ctrl_pending(external_);
		e = ::BIO_write(external_, (what + written), (size-written) );
		if( e < 0 )
			return -1;
		written += e;
	}
	int ret = ::SSL_read(ossl_, to, written);
	encrypted = (ret > 0) ? static_cast<std::size_t>(ret) : 0;
	return ret;
}

//tls_channel

tls_channel::tls_channel(session&& ssl,s_read_write_channel&& socket) noexcept:
	session_( std::forward<session>(ssl) ),
	socket_( std::forward<s_read_write_channel>(socket) )
{}

tls_channel::~tls_channel() noexcept
{}

std::size_t tls_channel::read(std::error_code& ec,uint8_t* const buff, std::size_t bytes) const noexcept
{
	std::size_t ret = socket_->read(ec, buff, bytes);
	if(ec)
		return 0;
	if( session_.decrypt(buff, ret, buff, ret ) < 0 ) {
		ec = std::make_error_code(std::errc::broken_pipe);
		return 0;
	}
	return ret;
}

std::size_t tls_channel::write(std::error_code& ec, const uint8_t* data,std::size_t size) const noexcept
{
	uint8_t buff[ 4096 ];
	std::size_t ret = size < 4096 ? size : 4096;
	std::size_t encrypted;
	if( session_.encrypt( data, ret, buff, encrypted) < 0 ) {
		ec = std::make_error_code(std::errc::broken_pipe);
		return 0;
	}
	transmit_buffer(ec, socket_, buff, encrypted);
	return ret;
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
			if( init_openssl() ) {
				client_context ctx = client_context::create(ec);
				if(!ec) {
					std::atexit( &service::destroy_openssl_atexit );
					ret = new (std::nothrow) service( std::move(ctx) );
					if(nullptr == ret)
						ec = std::make_error_code(std::errc::not_enough_memory);
				}
			}
			else
				ec = std::make_error_code( std::errc::protocol_not_supported );
			_instance.store(ret, std::memory_order_release);
		}
	}
	return ret;
}

service::service(client_context&& cl_ctx) noexcept:
	client_context_( std::forward<client_context>(cl_ctx) )
{}

service::~service() noexcept
{
	::ERR_remove_state(0);
	::ENGINE_cleanup();
	::CONF_modules_unload(1);
	::ERR_free_strings();
	::EVP_cleanup();
	::sk_SSL_COMP_free( ::SSL_COMP_get_compression_methods() );
	::CRYPTO_cleanup_all_ex_data();
}

s_read_write_channel service::new_client_connection(std::error_code& ec, s_read_write_channel&& socket) const noexcept
{
	session ssl = session::open(ec, client_context_ .get() );
	if(ec)
		return s_read_write_channel();
	// TLS handshake
	uint8_t buff[ 4096 ];
	ssize_t len;
	do {
		io_zerro_mem(buff, 4096);
		len = ssl.get_client_handshake_data(buff, 4096);
		if( (len < 0) && (SSL_ERROR_WANT_READ != ssl.get_error(len)) ) {
			ec = std::make_error_code(std::errc::connection_refused);
		} else {
			transmit_buffer(ec, socket, buff, len );
			if(ec)
				break;
			io_zerro_mem(buff, 4096);
			for(len = 0; !ec && len == 0; )
				len = socket->read(ec, buff, 4096);
			if(!ec) {
				len = ssl.set_server_handshake_data(buff, static_cast<uint16_t>(len) );
				if(len < 0)
					ec = std::make_error_code(std::errc::connection_refused);
			}
		}
	}
	while( !ec && !ssl.handshake_done() );
	if(ec)
		return s_read_write_channel();
	tls_channel *ret = nobadalloc<tls_channel>::construct(ec, std::move(ssl), std::move(socket) );
	return ec  ? s_read_write_channel() : s_read_write_channel(ret);
}


} // namespace secure

} // namespace io
