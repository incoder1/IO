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
#ifndef __IO_WIN_SSPI_CHANNEL_HPP_INCLUDED__
#define __IO_WIN_SSPI_CHANNEL_HPP_INCLUDED__

#include "config.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

// stdlib C++ headers
#include <atomic>

#ifndef SECURITY_WIN32
#	define SECURITY_WIN32 1
#endif // SECURITY_WIN32

// windows headers
#include <security.h>
#include <sspi.h>
#include <rpc.h>
#include <schnlsp.h>
#include <schannel.h>

// io headers
#include <channels.hpp>
#include "sockets.hpp"
#include "criticalsection.hpp"

namespace io {

namespace net {

namespace secure {

enum class errc: HRESULT
{
	success = SEC_E_OK,
	no_memory = SEC_E_INSUFFICIENT_MEMORY,
	internal_error = SEC_E_INTERNAL_ERROR,
	no_creds = SEC_E_NO_CREDENTIALS,
	not_owner = SEC_E_NOT_OWNER,
	no_ecryption_provider = SEC_E_SECPKG_NOT_FOUND,
	unknown_credentials = SEC_E_UNKNOWN_CREDENTIALS,
	incomplete_message = SEC_E_INCOMPLETE_MESSAGE,
	logon_denied = SEC_E_LOGON_DENIED,
	no_authenticating_authority = SEC_E_NO_AUTHENTICATING_AUTHORITY,
	target_unknown = SEC_E_TARGET_UNKNOWN,
	unsupported_function = SEC_E_UNSUPPORTED_FUNCTION
};

class IO_PUBLIC_SYMBOL error_category final: public std::error_category
{
private:
#ifndef  _MSC_VER
	friend std::error_code  make_error_code(errc ec) noexcept;
	friend std::error_condition  make_error_condition(errc err) noexcept;
#else
	friend IO_PUBLIC_SYMBOL std::error_code make_error_code(errc errc) noexcept;
	friend IO_PUBLIC_SYMBOL std::error_condition make_error_condition(errc err) noexcept;
#endif
	static inline const error_category* instance() noexcept;
	const char* cstr_message(int err) const;
public:
	constexpr error_category() noexcept:
		std::error_category()
	{}

	virtual ~error_category() = default;

	virtual const char* name() const noexcept override;

	virtual std::error_condition default_error_condition (int err) const noexcept override;

	virtual bool equivalent(const std::error_code& code, int condition) const noexcept override;

	virtual std::string message(int err_code) const override
	{
		return std::string(  this->cstr_message(err_code) );
	}
};


#ifndef  _MSC_VER
	IO_PUBLIC_SYMBOL std::error_code  make_error_code(errc ec) noexcept;
	IO_PUBLIC_SYMBOL std::error_condition  make_error_condition(errc err) noexcept;
#else
	std::error_code IO_PUBLIC_SYMBOL make_error_code(errc errc) noexcept;
	std::error_condition IO_PUBLIC_SYMBOL make_error_condition(errc err) noexcept;
#endif


struct credentials {
	credentials(const credentials&)  = delete;
	credentials& operator=(const credentials&)  = delete;
public:
	constexpr explicit credentials() noexcept:
		h_cred_({0,0})
	{}
	credentials(credentials&& rhs) noexcept:
		h_cred_( std::move(rhs.h_cred_) )
	{
		rhs.h_cred_ = {0,0};
	}
	credentials& operator=(credentials&& rhs) noexcept {
		credentials( std::forward<credentials>(rhs) ).swap( *this );
		return *this;
	}
	explicit credentials(::CredHandle&& hnd) noexcept:
		h_cred_( std::move(hnd) )
	{}
	::PCredHandle get() const noexcept
	{
        return const_cast<::PCredHandle>(&h_cred_);
	}
	~credentials() noexcept {
		if( check(h_cred_) )
			::FreeCredentialsHandle(&h_cred_);
	}
private:
	void swap(credentials& other) noexcept {
		::CredHandle tmp = {h_cred_.dwLower, h_cred_.dwUpper};
		h_cred_ = {other.h_cred_.dwLower,h_cred_.dwUpper};
		other.h_cred_ = {tmp.dwLower, tmp.dwUpper};
	}
	static inline constexpr bool check(const ::CredHandle& hnd) {
		return hnd.dwLower !=  0 &&  hnd.dwUpper != 0;
	}
private:
	::CredHandle h_cred_;
};

class connection_contex;
DECLARE_IPTR(connection_contex);

// connection_contex
class connection_contex final: public object
{
public:
	static constexpr std::size_t MAX_MESSAGE = 12000;

	static s_connection_contex new_context(std::error_code& ec, ::PCredHandle crds,
										s_read_write_channel&& sch) noexcept;

	bool client_handshake(std::error_code& ec) noexcept;

	virtual ~connection_contex() noexcept;
private:
	friend class nobadalloc<connection_contex>;
	connection_contex(::CtxtHandle cnthdn, scoped_arr<::BYTE>&& in, scoped_arr<::BYTE>&& out, s_read_write_channel&& raw) noexcept;
private:
	::CtxtHandle h_cntx_;
	s_read_write_channel raw_;
	scoped_arr<::BYTE> in_;
	scoped_arr<::BYTE> out_;
};


class IO_PUBLIC_SYMBOL service final {
	service(const service&) = delete;
	service& operator=(const service&) = delete;
public:
	static const service* instance(std::error_code& ec) noexcept;
	~service() noexcept;
	s_read_write_channel new_client_connection(std::error_code& ec, s_read_write_channel&& socket) const noexcept;
private:

	static credentials sspi_init(std::error_code& ec) noexcept;

	static void release_on_exit() noexcept;

	explicit service(credentials&& creds) noexcept;
	static std::atomic<service*> _instance;
	static critical_section _init_mtx;

	credentials creds_;
};


} // secure

} // net

} // namespace io

namespace std
{
  template <>
    struct is_error_condition_enum<io::net::secure::errc>
    : true_type {};

  inline std::error_condition make_error_condition(io::net::secure::errc e) noexcept
  {
	return io::net::secure::make_error_condition(e);
  }

}

#endif // __IO_WIN_SSPI_CHANNEL_HPP_INCLUDED__
