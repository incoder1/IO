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
#include "secure_socket.hpp"


namespace io {

namespace net {

// certificate
const wchar_t* certificate::SUBJECT = L"CN=X509 Client Certificate";
const wchar_t* certificate::KEY_CONTAINER_NAME = L"Container";

::PCCERT_CONTEXT certificate::generate(const ::HCRYPTPROV h_prov,const ::PCERT_NAME_BLOB sib) noexcept
{
	::CRYPT_KEY_PROV_INFO kpi;
	io_zerro_mem( &kpi, sizeof(kpi) );
	kpi.pwszContainerName = const_cast<::LPWSTR>( KEY_CONTAINER_NAME );
	kpi.pwszProvName =  const_cast<::LPWSTR>(MS_DEF_PROV_W);
	kpi.dwProvType = PROV_RSA_FULL;
	kpi.dwFlags = CERT_SET_KEY_CONTEXT_PROP_ID;
	kpi.dwKeySpec = AT_KEYEXCHANGE;
	// SID with current time + 1year
	::SYSTEMTIME et;
	::GetSystemTime(&et);
	et.wYear += 1;
	::CERT_EXTENSIONS exts;
	io_zerro_mem( &exts, sizeof(exts) );
	return ::CertCreateSelfSignCertificate(h_prov, sib, 0, &kpi,nullptr,nullptr, &et, &exts);
}

::HCRYPTPROV certificate::new_provider() noexcept
{
	::HCRYPTPROV ret = 0;
	if (!::CryptAcquireContextW(
	            &ret,
	            KEY_CONTAINER_NAME,
	            MS_DEF_PROV_W,
	            PROV_RSA_FULL,
	            CRYPT_NEWKEYSET | CRYPT_MACHINE_KEYSET)
	   ) {
		::HRESULT err = ::GetLastError();
		if (NTE_EXISTS == err) {
			if (!::CryptAcquireContextW(&ret,
			                            KEY_CONTAINER_NAME,
			                            MS_DEF_PROV_W,
			                            PROV_RSA_FULL,
			                            CRYPT_MACHINE_KEYSET)
			   )
				return 0;
		} else
			return 0;
	}
	return ret;
}

::HCRYPTKEY certificate::new_key_pair(const ::HCRYPTPROV prov) noexcept
{
	::HCRYPTKEY ret;
	if( !CryptGenKey(prov, AT_KEYEXCHANGE, CRYPT_EXPORTABLE, &ret) )
		return 0;
	return ret;
}

s_certificate certificate::create(std::error_code &ec) noexcept
{
	::CERT_NAME_BLOB sib = { 0, nullptr };
	uint8_t cb[1000];
	io_zerro_mem(cb, 1000);
	sib.cbData = 1000;
	sib.pbData = cb;
	if ( !::CertStrToNameW(
	            X509_ASN_ENCODING,
	            SUBJECT, 0, 0,
	            sib.pbData, &sib.cbData, nullptr)
	   ) {
		ec.assign( ::GetLastError(), std::system_category() );
		return s_certificate();
	}
	::HCRYPTPROV prov = new_provider();
	if( !prov ) {
		ec.assign( ::GetLastError(), std::system_category() );
		return s_certificate();
	}
	// Generate KeyPair
	::HCRYPTKEY key_pair = new_key_pair(prov);
	if(!key_pair) {
		::CryptReleaseContext(prov, 0);
		ec.assign( ::GetLastError(), std::system_category() );
		return s_certificate();
	}
	::PCCERT_CONTEXT ret = generate(prov, &sib);
	if( ! ::CryptFindCertificateKeyProvInfo(ret,CRYPT_FIND_MACHINE_KEYSET_FLAG,nullptr) ) {
		ec.assign( ::GetLastError(), std::system_category() );
		::CryptDestroyKey(key_pair);
		::CryptReleaseContext(prov, 0);
		return s_certificate();
	}
	::CryptDestroyKey(key_pair);
	::CryptReleaseContext(prov, 0);
	return s_certificate( nobadalloc<certificate>::construct(ec, ret) );
}

certificate::certificate(::PCCERT_CONTEXT context) noexcept:
	object(),
	context_(context)
{}

certificate::~certificate() noexcept
{
	::CertFreeCertificateContext(context_);
}

const_string certificate::info(std::error_code &ec) noexcept
{
	char tmp[1024];
	io_zerro_mem(tmp, 1024);
	if( ::CertGetNameStringA(context_,
	                         CERT_NAME_FRIENDLY_DISPLAY_TYPE,
	                         CERT_NAME_SEARCH_ALL_NAMES_FLAG,
	                         nullptr,
	                         tmp,
	                         1024) ) {
		const_string ret( tmp );
		if(ret.empty())
			ec = std::make_error_code( std::errc::not_enough_memory);
		return ret;
	}
	return const_string();
}

// encryptor
static constexpr bool check_context_initialized(::SECURITY_STATUS ss) noexcept {
	return ss != SEC_I_CONTINUE_NEEDED &&
		   ss != SEC_E_INCOMPLETE_MESSAGE &&
		   ss != SEC_I_INCOMPLETE_CREDENTIALS;
}

static std::size_t write_all(std::error_code& ec, const s_read_write_channel& ch, const void* b, std::size_t s) noexcept
{
	const uint8_t *i = static_cast<const uint8_t*>(b);
	std::size_t left = s;
	std::size_t ret = 0, written;
	while(!ec) {
		written = ch->write(ec, i, left);
		if(ec || 0 == written)
			break;
		ret += written;
		left -= written;
		i += written;
	}
	return ret;
}

static std::size_t read_all(std::error_code& ec, const s_read_write_channel& ch, void* const b, std::size_t s) noexcept
{
	uint8_t *i = static_cast<uint8_t*>(b);
	std::size_t left = s;
	std::size_t ret = 0, read;
	while(!ec) {
        read = ch->read(ec, i, left);
        if(ec || 0 == read)
			break;
        ret += read;
		left -= read;
		i += read;
	}
	return ret;
}

s_encryptor encryptor::create(std::error_code &ec,
							const s_certificate& cert,
							s_read_write_channel&& socket) noexcept
{
	::CredHandle h_cred;
	::SCHANNEL_CRED schannel_cred;
	std::memset(&schannel_cred, 0, sizeof(schannel_cred) );
	schannel_cred.dwVersion = SCHANNEL_CRED_VERSION;
	//schannel_cred.dwFlags = SCH_CRED_NO_DEFAULT_CREDS;
	schannel_cred.dwFlags = SCH_CRED_NO_DEFAULT_CREDS |
							SCH_CRED_NO_SYSTEM_MAPPER |
							SCH_CRED_REVOCATION_CHECK_CHAIN;
	::PCCERT_CONTEXT cert_context = cert->get_context();
	schannel_cred.cCreds     = 1;
	schannel_cred.paCred = &cert_context;
	::SECURITY_STATUS ss = ::AcquireCredentialsHandleW(
							   nullptr,
							   const_cast<SEC_WCHAR*>(SCHANNEL_NAME_W),
							   SECPKG_CRED_OUTBOUND,
							   0,&schannel_cred,0,0,&h_cred,0
							);
	if (FAILED(ss)) {
		ec = std::make_error_code( std::errc::not_supported);
		return s_encryptor();
	}
	// initalize context
	ss = SEC_I_CONTINUE_NEEDED;
	scoped_arr<uint8_t> t(0x11000);
	scoped_arr<::SecBuffer> bufsi(100);
	scoped_arr<::SecBuffer> bufso(100);
	static constexpr ::DWORD INIT_SSPI_FLAGS =
				  ISC_REQ_SEQUENCE_DETECT   |
                  ISC_REQ_REPLAY_DETECT     |
                  ISC_REQ_CONFIDENTIALITY   |
                  ISC_RET_EXTENDED_ERROR    |
                  ISC_REQ_ALLOCATE_MEMORY   |
                  ISC_REQ_STREAM            |
                  ISC_REQ_MANUAL_CRED_VALIDATION;
	::SecBufferDesc sb_in = {0,0,nullptr};
	::SecBufferDesc sb_out = {0,0,nullptr};
	std::size_t transfered = 0;
	bool init_context = false;
	::DWORD SSPI_flags, SSPI_out_flags;
	::CtxtHandle h_context = {0,0};
	std::size_t pt = 0;
	wchar_t target_name[1024]; // 2k
	std::memset(target_name, 0, sizeof(target_name) );
	while( !check_context_initialized(ss) ) {
		SSPI_flags = INIT_SSPI_FLAGS;
		if(init_context) {
			transfered = socket->read(ec, t.get(), t.len() );
			if(ec || 0 == transfered) {
				if(!ec)
					ec = std::make_error_code( std::errc::io_error);
				return s_encryptor();
			}
			pt += transfered;
			// Put this data into the InitializeSecurityContex buffer
			bufsi[0].BufferType = SECBUFFER_TOKEN;
			bufsi[0].cbBuffer = pt;
			bufsi[0].pvBuffer = t.get();
			bufsi[1].BufferType = SECBUFFER_EMPTY;
			bufsi[1].cbBuffer = 0;
			bufsi[1].pvBuffer = 0;
			sb_in.ulVersion = SECBUFFER_VERSION;
			sb_in.pBuffers = bufsi.get();
			sb_in.cBuffers = 2;
			bufso[0].pvBuffer  = NULL;
			bufso[0].BufferType= SECBUFFER_TOKEN;
			bufso[0].cbBuffer  = 0;
			sb_out.cBuffers      = 1;
			sb_out.pBuffers      = bufso.get();
			sb_out.ulVersion     = SECBUFFER_VERSION;
		} else {
			// Initialize sbout
		    bufso[0].pvBuffer   = nullptr;
			bufso[0].BufferType = SECBUFFER_TOKEN;
			bufso[0].cbBuffer   = 0;
			sb_out.ulVersion = SECBUFFER_VERSION;
			sb_out.cBuffers = 1;
			sb_out.pBuffers = bufso.get();
		}
		SSPI_out_flags = 0;
		ss = ::InitializeSecurityContextW(
			&h_cred,
			init_context ? &h_context : nullptr,
			target_name,
			SSPI_flags,
			0,
			0,//SECURITY_NATIVE_DREP,
			init_context ? &sb_in : nullptr,
			0,
			init_context ? 0 : &h_context,
			&sb_out,
			&SSPI_out_flags,
			0);
		// allow more
		if (ss == SEC_E_INCOMPLETE_MESSAGE)
			continue;
		pt = 0;
		if (FAILED(ss)) {
			ec = std::make_error_code(std::errc::io_error);
			return s_encryptor();
		}
		if( !init_context && ss != SEC_I_CONTINUE_NEEDED) {
			ec = std::make_error_code( std::errc::function_not_supported );
			return s_encryptor();
 		}
		// Send the data we got to the remote part
		if (!init_context) {
			transfered = write_all(ec,socket,bufso[0].pvBuffer,bufso[0].cbBuffer);
			::FreeContextBuffer(bufso[0].pvBuffer);
			if (ec || transfered != bufso[0].cbBuffer) {
				ec = std::make_error_code( std::errc::io_error );
				return s_encryptor();
			}
			init_context = true;
			continue;
		}
		// Pass data to the remote site
		transfered = write_all(ec,socket,bufso[0].pvBuffer,bufso[0].cbBuffer);
		if( ec || transfered != bufso[0].cbBuffer) {
			::FreeContextBuffer(bufso[0].pvBuffer);
			return s_encryptor();
		}
		::FreeContextBuffer(bufso[0].pvBuffer);
		if (ss == S_OK)
			break;
	}
	encryptor* ret = nobadalloc<encryptor>::construct(ec,
						h_context,
						sb_in,
						sb_out,
						std::forward<s_read_write_channel>( socket )
					);
	return nullptr != ret ? s_encryptor(ret) : s_encryptor();
}

encryptor::encryptor(::CtxtHandle h_context,
			::SecBufferDesc in,
			::SecBufferDesc out,
			s_read_write_channel&& socket) noexcept:
	read_write_channel(),
	h_context_(h_context),
	in_buf_desc_(in),
	out_buf_desc_(out),
	socket_( std::forward<s_read_write_channel>(socket) )
{
}

std::size_t encryptor::read(std::error_code& ec,uint8_t* const buff, std::size_t bytes) const noexcept
{
	::SecPkgContext_StreamSizes sizes;
	::SECURITY_STATUS ss = ::QueryContextAttributes(&h_context_, SECPKG_ATTR_STREAM_SIZES, &sizes);
	if ( FAILED(ss) ) {
		ec = std::make_error_code( std::errc::io_error );
		return 0;
	}
	std::size_t ret = 0;
	std::size_t encrypted = 0;
	::SecBuffer buffers[5];
	std::memset(buffers, 0 , sizeof(buffers) );
	::SecBuffer* data_buffer;
	scoped_arr<uint8_t> mmsg( sizes.cbMaximumMessage * 10);
	if(!mmsg) {
		ec = std::make_error_code( std::errc::not_enough_memory );
		return 0;
	}
	unsigned int dw_message = sizes.cbMaximumMessage;

	return 0;
}

std::size_t encryptor::write(std::error_code& ec, const uint8_t* buff,std::size_t size) const noexcept
{
	return 0;
}


} // namespace net

} // namespace io
