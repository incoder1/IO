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
	static constexpr const long flags = SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3;
	::SSL_CTX *ctx = ::SSL_CTX_new( ::TLSv1_2_client_method() );
	if(nullptr == ctx) {
		ec = std::make_error_code( std::errc::protocol_not_supported );
		return client_context();
	}
	::SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, NULL);
	::SSL_CTX_set_options(ctx, flags);
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
	::BIO_new_bio_pair(&interal,  memory_traits::page_size(), &external, memory_traits::page_size());
	if(nullptr == interal || nullptr == external ) {
		ec = std::make_error_code(std::errc::not_enough_memory);
		::SSL_free(ossl);
		return session();
	}

	::BIO_set_mem_eof_return(interal, EOF);
	::BIO_set_mem_eof_return(external, EOF);

	::SSL_set_bio(ossl, interal, interal);
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
		::BIO_free(internal_);
		::BIO_free(external_);
		::SSL_free(ossl_);
	}
}

std::size_t session::connect(uint8_t* const buff, uint16_t size) noexcept
{
	::BIO_do_connect(internal_);
	return static_cast<std::size_t>( ::BIO_read(external_, buff, size) );
}

std::size_t session::handshake(const uint8_t* buff, uint16_t size) noexcept
{
	::BIO_write(external_, buff, size);
	::BIO_do_handshake(internal_);
}

int session::decrypt(const uint8_t *what, uint8_t* const to, uint16_t size) const noexcept
{
	int decrypted = ::BIO_write(external_, static_cast<const void*>(what), size);
	return decrypted > 0 ? ::SSL_read(ossl_, static_cast<void*>(to), decrypted ) : decrypted;
}

int session::encrypt(const uint8_t *what, uint8_t* const to, uint16_t size) const noexcept
{
	int encrypted = ::SSL_write(ossl_, static_cast<const void*>(what), size);
	return encrypted > 0 ? ::BIO_read(external_, static_cast<void*>(to), encrypted) : encrypted;
}

//tls_channel

tls_channel::tls_channel(session&& ssl,s_read_write_channel&& raw) noexcept:
	session_( std::forward<session>(ssl) ),
	raw_( std::forward<s_read_write_channel>(raw) )
{}

tls_channel::~tls_channel() noexcept
{}

std::size_t tls_channel::read(std::error_code& ec,uint8_t* const buff, std::size_t bytes) const noexcept
{
	static constexpr const std::size_t MAX = std::numeric_limits<uint16_t>::max();
	std::size_t to_read  = bytes < MAX ? bytes: MAX;
	uint8_t *tmp = memory_traits::calloc_temporary<uint8_t>(to_read);
	if(nullptr == tmp) {
		ec = std::make_error_code(std::errc::not_enough_memory);
		return 0;
	}
	std::size_t read = raw_->read(ec, tmp, to_read);
	if( 0 == read || ec ) {
		memory_traits::free_temporary(tmp);
		return 0;
	}
	std::size_t ret = static_cast<std::size_t>( session_.decrypt(tmp, buff, static_cast<uint16_t>(read) ) );
	memory_traits::free_temporary(tmp);
	return ret;
}

std::size_t tls_channel::write(std::error_code& ec, const uint8_t* buff,std::size_t size) const noexcept
{
	static constexpr const std::size_t MAX = std::numeric_limits<uint16_t>::max();
	std::size_t chunk  = size < MAX ? size : MAX;
	uint8_t *tmp = memory_traits::calloc_temporary<uint8_t>(chunk);
	if(nullptr == tmp) {
		ec = std::make_error_code(std::errc::not_enough_memory);
		return 0;
	}
	int ret = session_.encrypt(buff, tmp, chunk);
	transmit_buffer(ec, raw_, tmp, static_cast<std::size_t>(ret) );
	memory_traits::free_temporary(tmp);
	return ec ? 0 : static_cast<std::size_t>(ret);
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
	uint8_t buff[ 4096 ];
	std::size_t len = ssl.handshake(buff, 4096);
	transmit_buffer(ec, socket, buff, len );
	if(ec)
		return s_read_write_channel();
	tls_channel *ret = nobadalloc<tls_channel>::construct(ec, std::move(ssl), std::move(socket) );
	return ec  ? s_read_write_channel() : s_read_write_channel(ret);
}


} // namespace secure

} // namespace io
