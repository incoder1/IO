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
	inline ::PCCERT_CONTEXT get_context() const noexcept
	{
		return context_;
	}
	~certificate() noexcept;
private:
	::PCCERT_CONTEXT context_;
};


class encryptor;
DECLARE_IPTR(encryptor);

class IO_PUBLIC_SYMBOL encryptor:public read_write_channel
{
public:
	static s_encryptor create(std::error_code &ec,
							const s_certificate& cert,
							s_read_write_channel&& socket) noexcept;
	virtual std::size_t read(std::error_code& ec,uint8_t* const buff, std::size_t bytes) const noexcept override;
	virtual std::size_t write(std::error_code& ec, const uint8_t* buff,std::size_t size) const noexcept override;
private:
	friend class nobadalloc<encryptor>;
	encryptor(::CtxtHandle h_context,
			::SecBufferDesc in,
			::SecBufferDesc out,
			s_read_write_channel&& socket) noexcept;
private:
	mutable ::CtxtHandle h_context_;
	::SecBufferDesc in_buf_desc_;
	::SecBufferDesc out_buf_desc_;
	s_read_write_channel socket_;
};

} // namespace net

} // namespace io

#endif // __IO_WIN_SECURE_SOCKET_HPP_INCLUDED__
