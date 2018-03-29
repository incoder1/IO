#include "stdafx.hpp"
#include "tls/openssl_secure_channel.hpp"

#ifdef __IO_WINDOWS_BACKEND__

#	ifdef __MINGW64__
#		include <prsht.h>
#	endif // defined

#	include <wincrypt.h>
#	include <cryptuiapi.h>

#	ifdef _MSC_VER
#		pragma comment (lib, "crypt32.lib")
#		pragma comment (lib, "cryptui.lib")
#	endif // _MSC_VER

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

static bool initialze_context_from_sspi(::SSL_CTX* const sslctx) noexcept
{
	static constexpr const wchar_t* SYS_NAME = L"HCURRENT_USER_STORE";
	::HCERTSTORE sspi_store = ::CertOpenStore(
								  CERT_STORE_PROV_SYSTEM, 0, 0,
								  CERT_SYSTEM_STORE_CURRENT_USER,
								  SYS_NAME);
	if(NULL == sspi_store)
		return false;
	::X509_STORE *ossl_store  = ::SSL_CTX_get_cert_store( sslctx );
	for(::PCCERT_CONTEXT sspi_cert = nullptr;;) {
		sspi_cert = ::CertEnumCertificatesInStore(sspi_store, sspi_cert);
		if(nullptr == sspi_cert)
			break;
		::BYTE *raw = sspi_cert->pbCertEncoded;
		::DWORD raw_size = sspi_cert->cbCertEncoded;
		::X509 *ossl_cert = ::d2i_X509(nullptr, const_cast<const unsigned char**>(&raw), raw_size);
		if(nullptr != ossl_cert)
			::X509_STORE_add_cert(ossl_store, ossl_cert);
	}
	::CertCloseStore(sspi_store, 0);
	return true;
}

#endif // __IO_WINDOWS_BACKEND__

// client_context

client_context client_context::create(std::error_code& ec) noexcept
{
	::SSL_CTX *ctx = ::SSL_CTX_new( ::TLSv1_2_client_method() );
	if(nullptr == ctx) {
		ec = std::make_error_code( std::errc::protocol_not_supported );
		return client_context();
	}
#ifdef __IO_WINDOWS_BACKEND__
	if( !initialze_context_from_sspi(ctx) ) {
		ec = std::make_error_code( std::errc::protocol_not_supported );
		return client_context();
	}
#endif // __IO_WINDOWS_BACKEND__
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
    ::BIO *rbuf = ::BIO_new( ::BIO_s_mem() );
    ::BIO *wbuf = ::BIO_new( ::BIO_s_mem() );
    if(nullptr == rbuf || nullptr == wbuf ) {
		ec = std::make_error_code(std::errc::not_enough_memory);
		::SSL_free(ossl);
		return session();
    }

    ::BIO_set_mem_eof_return(rbuf, 0);
	::BIO_set_mem_eof_return(wbuf, 0);

	::SSL_set_bio(ossl, rbuf, wbuf);
	return session(ossl, rbuf, wbuf);
}

session::session(::SSL* const ossl, ::BIO* const rbuf,::BIO* const wbub) noexcept:
	ossl_(ossl),
	rbuf_(rbuf),
	wbuf_(wbub)
{}

session::session(session&& c) noexcept:
  session(c.ossl_,c.rbuf_,c.wbuf_)
{
	c.ossl_ = nullptr;
	c.rbuf_ = nullptr;
	c.wbuf_ = nullptr;
}

session::~session() noexcept
{
	if( nullptr != ossl_ ) {
        ::BIO_free(rbuf_);
		::BIO_free(wbuf_);
		::SSL_free(ossl_);
	}
}

bool session::hand_stake(std::error_code& ec, const s_read_write_channel& raw) noexcept
{
	return false;
}

std::size_t session::read(std::error_code& ec, uint8_t * const to, std::size_t bytes) noexcept
{
	return 0;
}

std::size_t session::write(std::error_code& ec, const uint8_t *what, std::size_t length) noexcept
{
	return 0;
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
	ec = std::make_error_code(std::errc::protocol_not_supported);
    return s_read_write_channel();
}


} // namespace secure

} // namespace io
