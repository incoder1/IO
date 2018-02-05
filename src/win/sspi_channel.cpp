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
#include "stdafx.hpp"
#include "sspi_channel.hpp"

#ifndef UNISP_NAME_W
#	define UNISP_NAME_W L"Microsoft Unified Security Protocol Provider"
#endif // UNISP_NAME_W

#ifndef SECPKG_CRED_OUTBOUND
#	define SECPKG_CRED_OUTBOUND 0x00000002
#endif // SECPKG_CRED_OUTBOUND

#ifndef SP_PROT_SSL2_CLIENT
#define SP_PROT_SSL2_CLIENT             0x00000008
#endif

#ifndef SP_PROT_SSL3_CLIENT
#define SP_PROT_SSL3_CLIENT             0x00000008
#endif

#ifndef SP_PROT_TLS1_CLIENT
#define SP_PROT_TLS1_CLIENT             0x00000080
#endif

#ifndef SP_PROT_TLS1_0_CLIENT
#define SP_PROT_TLS1_0_CLIENT           SP_PROT_TLS1_CLIENT
#endif

#ifndef SP_PROT_TLS1_1_CLIENT
#define SP_PROT_TLS1_1_CLIENT           0x00000200
#endif

#ifndef SP_PROT_TLS1_2_CLIENT
#define SP_PROT_TLS1_2_CLIENT           0x00000800
#endif

#ifndef SECBUFFER_ALERT
#define SECBUFFER_ALERT                 17
#endif

#define CURL_SCHANNEL_BUFFER_INIT_SIZE   4096
#define CURL_SCHANNEL_BUFFER_FREE_SIZE   1024

#ifndef CALG_SHA_256
#  define CALG_SHA_256 0x0000800c
#endif

#include <cstdlib>

namespace io {

namespace net {

namespace secure {

// error_category
const error_category* error_category::instance() noexcept
{
	static error_category _instance;
	return &_instance;
}

const char* error_category::name() const noexcept
{
        return "Windows SChannel/SSPI error condition";
}

std::error_condition error_category::default_error_condition (int err) const noexcept
{
	return std::error_condition(err, *instance() );
}

bool error_category::equivalent(const std::error_code& code, int condition) const noexcept
{
	return static_cast<int>(this->default_error_condition(code.value()).value()) == condition;
}

const char* error_category::cstr_message(int err_code) const
{
	errc ec = static_cast<errc>(err_code);
	switch( ec ) {
	case errc::success:
		return "No SSPI error";
	case errc::no_memory:
		return "There is not enough memory available to complete the requested action";
	case errc::no_creds:
		return "No credentials are available in the security package";
	case errc::not_owner:
        return "The caller of the function does not have the necessary credentials";
	case errc::no_ecryption_provider:
        return "The requested security package does not exist";
	case errc::unknown_credentials:
        return "The credentials supplied to the package were not recognized";
	case errc::incomplete_message:
		return "Data for the whole message was not read from the wire";
	case errc::logon_denied:
		return "The logon failed";
	case errc::no_authenticating_authority:
		return "No authority could be contacted for authentication. The domain name of the authenticating party could be wrong, the domain could be unreachable, or there might have been a trust relationship failure";
	case errc::target_unknown:
		return "The target was not recognized";
	case errc::unsupported_function:
		return "A context attribute flag that is not valid";
	case errc::internal_error:
	default:
		return "An error occurred that did not map to an SSPI error code";
	}
}

std::error_code IO_PUBLIC_SYMBOL make_error_code(errc ec) noexcept
{
	return std::error_code( static_cast<int>(ec), *(error_category::instance()) );
}

std::error_condition IO_PUBLIC_SYMBOL make_error_condition(errc err) noexcept
{
	return std::error_condition(static_cast<int>(err), *(error_category::instance()) );
}

// connection_contex

s_connection_contex connection_contex::new_context(std::error_code& ec, ::PCredHandle crds, s_read_write_channel&& sch) noexcept
{
	static constexpr ::ULONG CLIENT_FLAGS =  ISC_REQ_SEQUENCE_DETECT |
                                    ISC_REQ_REPLAY_DETECT   |
                                    ISC_REQ_CONFIDENTIALITY |
                                    ISC_RET_EXTENDED_ERROR  |
                                    ISC_REQ_ALLOCATE_MEMORY |
                                    ISC_REQ_MUTUAL_AUTH 	|
                                    ISC_REQ_STREAM;

	scoped_arr<::BYTE> in(MAX_MESSAGE);
	scoped_arr<::BYTE> out(MAX_MESSAGE);

	if(!in || !out) {
		ec = std::make_error_code(std::errc::not_enough_memory);
		return s_connection_contex();
	}

	::SecBuffer outb[1] = { {MAX_MESSAGE,SECBUFFER_TOKEN,out.get()} };
	::SecBufferDesc outBufs;
	outBufs.ulVersion = SECBUFFER_VERSION;
	outBufs.cBuffers = 1;
	outBufs.pBuffers = outb;

	::ULONG attributes;

	::CtxtHandle ret;

	::TimeStamp timeOut;
	timeOut.QuadPart = 1200;

	::SECURITY_STATUS ss = ::InitializeSecurityContextW(
		crds,
		nullptr, nullptr,
		CLIENT_FLAGS,
		0, 0,
		nullptr,
		0,
		&ret,
		&outBufs,
		&attributes,
		&timeOut);

	switch(ss) {
	case SEC_E_OK:
    case SEC_E_INCOMPLETE_MESSAGE:
	case SEC_I_INCOMPLETE_CREDENTIALS:
    case SEC_I_CONTINUE_NEEDED:
	case SEC_I_COMPLETE_NEEDED:
    case SEC_I_COMPLETE_AND_CONTINUE:
    	break;
	default:
        ec = make_error_code( static_cast<errc>(ss) );
		return s_connection_contex();
	}

	connection_contex *ctx = nobadalloc<connection_contex>::construct(ec, ret, std::move(in), std::move(out), std::forward<s_read_write_channel>(sch) );
	if(!ec && ctx->client_handshake(ec)) {
		return s_connection_contex( ctx );
	}
	return s_connection_contex();
}

connection_contex::connection_contex(::CtxtHandle cnthdn, scoped_arr<::BYTE>&& in, scoped_arr<::BYTE>&& out, s_read_write_channel&& raw) noexcept:
	object(),
	h_cntx_(cnthdn),
    in_( std::forward<scoped_arr<::BYTE> >(in) ),
    out_( std::forward<scoped_arr<::BYTE> >(out) ),
	raw_( std::forward<s_read_write_channel>(raw) )
{}

connection_contex::~connection_contex() noexcept
{
	::DeleteSecurityContext(&h_cntx_);
}

bool connection_contex::client_handshake(std::error_code& ec) noexcept
{
	::SECURITY_STATUS ss = SEC_E_INCOMPLETE_MESSAGE;
	std::size_t read;
	while( ss == SEC_E_INCOMPLETE_MESSAGE || ss ==  SEC_I_CONTINUE_NEEDED ) {
		// send client creds
        io::transmit_buffer( ec, raw_, out_.get(), out_.len() );
		// read server response
		read = raw_->read(ec, in_.get(), in_.len() );
		if(ec)
			return false;
	}

	return false;
}

//service
std::atomic<service*> service::_instance(nullptr);
critical_section service::_init_mtx;

void service::release_on_exit() noexcept
{
	service* tmp = _instance.load(std::memory_order_consume);
	if(nullptr != tmp) {
		delete tmp;
		_instance.store(nullptr, std::memory_order_release);
	}
}

credentials service::sspi_init(std::error_code& ec) noexcept
{
    ::CredHandle h_cred = {0,0};
    ::TimeStamp         lifetime;
	::SECURITY_STATUS ss;
	ss = ::AcquireCredentialsHandleW (
            NULL,
            const_cast<SEC_WCHAR*>(UNISP_NAME_W),
			SECPKG_CRED_OUTBOUND,
            NULL,NULL,NULL,NULL,
            &h_cred, &lifetime);
	if(SEC_E_OK != ss) {
		ec = make_error_code( static_cast<errc>(ss) );
		return credentials();
	}
	return credentials( std::move(h_cred) );
}

const service* service::instance(std::error_code& ec) noexcept
{
	service* ret = _instance.load(std::memory_order_consume);
	if(nullptr == ret) {
		lock_guard lock(_init_mtx);
		ret = _instance.load(std::memory_order_consume);
		if(nullptr == ret) {
			std::atexit(&service::release_on_exit);
			credentials creds = sspi_init(ec);
			if(!ec)
				ret = new (std::nothrow) service( std::move(creds) );
			_instance.store(ret, std::memory_order_release);
		}
	}
	return ret;
}

service::service(credentials&& creds) noexcept:
	creds_( std::forward<credentials>(creds) )
{}

service::~service() noexcept
{
}


s_read_write_channel service::new_client_connection(std::error_code& ec, s_read_write_channel&& sch) const noexcept
{
	s_connection_contex ctx = connection_contex::new_context(ec, creds_.get(), std::forward<s_read_write_channel>(sch) );
	return ec ? s_read_write_channel() : s_read_write_channel();
}

} // secure

} // net

} // namespace io
