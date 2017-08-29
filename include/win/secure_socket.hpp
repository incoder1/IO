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
#ifndef __IO_WIN_SECURE_SOCKET_HPP_INCLUDED__
#define __IO_WIN_SECURE_SOCKET_HPP_INCLUDED__

#include "config.hpp"

#ifndef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#ifndef SECURITY_WIN32
#	define SECURITY_WIN32
#endif // SECURITY_WIN32

#include <winsock2.h>
#include <ws2tcpip.h>
#include <wincrypt.h>
#include <security.h>
#include <schnlsp.h>

#include <memory>
#include "conststring.hpp"
#include "wsaerror.hpp"

namespace io {

namespace net {

// Certificate (X509)
class certificate;
DECLARE_IPTR(certificate);

class IO_PUBLIC_SYMBOL certificate final:public object {
	certificate(const certificate&) = delete;
	certificate operator=(const certificate&) = delete;
private:
	friend class nobadalloc<certificate>;
	static const wchar_t* SUBJECT;
	static const wchar_t* KEY_CONTAINER_NAME;
	certificate(::PCCERT_CONTEXT context) noexcept;
	static ::HCRYPTPROV new_provider() noexcept;
	static ::HCRYPTKEY new_key_pair(const ::HCRYPTPROV prov) noexcept;
	static ::PCCERT_CONTEXT generate(const ::HCRYPTPROV h_prov,const ::PCERT_NAME_BLOB sib) noexcept;
public:
	static s_certificate create(std::error_code &ec) noexcept;
	const_string info(std::error_code &ec) noexcept;
	inline ::PCCERT_CONTEXT get() const noexcept
	{
		return context_;
	}
	~certificate() noexcept;
private:
	::PCCERT_CONTEXT context_;
};

namespace enrypt {

class encryptor:public object
{
public:
private:

};

} // namespace enrypt


} // namespace net

} // namespace io

#endif // __IO_WIN_SECURE_SOCKET_HPP_INCLUDED__
