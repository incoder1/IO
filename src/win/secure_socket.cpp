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

// secure_socket


} // namespace net

} // namespace io
