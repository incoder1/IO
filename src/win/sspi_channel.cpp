#include "sspi_channel.hpp"

#ifndef UNISP_NAME
#define UNISP_NAME L"Microsoft Unified Security Protocol Provider"
#endif

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

namespace io {

namespace net {

namespace secure {

static inline bool sec_success(::SECURITY_STATUS stat)
{
	return stat >= 0;
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
    static wchar_t      package_name[1024];
    ::CredHandle h_cred;
    ::TimeStamp         lifetime;

	if( !sec_success( ::AcquireCredentialsHandleW (
            NULL, package_name, SECPKG_CRED_OUTBOUND,
            NULL,NULL,NULL,NULL,
            &h_cred, &lifetime) )
	  ) {
		ec = std::make_error_code(std::errc::broken_pipe);
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

connection_contex service::client_handshake(std::error_code& ec, s_read_write_channel&& socket) noexcept
{
	scoped_arr<uint8_t> in(connection_contex::MAX_MESSAGE);
	if(!in) {
		ec = std::make_error_code( std::errc::not_enough_memory );
		return connection_contex();
	}
	scoped_arr<uint8_t> out(connection_contex::MAX_MESSAGE);
	if(!out) {
		ec = std::make_error_code( std::errc::not_enough_memory );
		return connection_contex();
	}
}

s_read_write_channel service::new_client_connection(std::error_code& ec, s_read_write_channel&& socket) const noexcept
{

	return s_read_write_channel();
}

} // secure

} // net

} // namespace io
