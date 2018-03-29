// Network support is not yet complete
// Able to took an XSD using  https TLS 1.2

#if defined(_WIN32) || defined(_WIN64)

#if _WIN32_WINNT < _WIN32_WINNT_VISTA
#	undef _WIN32_WINNT
#	define _WIN32_WINNT _WIN32_WINNT_VISTA
#else
#	define _WIN32_WINNT _WIN32_WINNT_VISTA
#endif // _WIN32_WINNT

#endif // _WIN32

#define IO_TLS_PROVIDER_GNUTSL

#include <errorcheck.hpp>
#include <network.hpp>

#include <net/uri.hpp>
#include <net/http_client.hpp>
#include <net/secure_channel.hpp>

#ifdef __IO_WINDOWS_BACKEND__

static void log(const char* data,const std::size_t bytes)
{
	static ::HANDLE hcout = ::GetStdHandle(STD_OUTPUT_HANDLE);
	::DWORD written;
    ::WriteFile(hcout, data, bytes, &written, nullptr);
}

#else

static void log(const char* data,const std::size_t bytes)
{
	::write(stdout, data, bytes);
}

#endif // __IO_WINDOWS_BACKEND__


int main()
{
	using namespace io::net;
	std::error_code ec;
	//s_uri url = uri::parse(ec, "http://www.springframework.org/schema/beans/spring-beans-4.2.xsd");
	s_uri url = uri::parse(ec, "https://www.springframework.org/beans/spring-beans-4.2.xsd");
	io::check_error_code(ec);

	io::check_error_code(ec);
	std::printf("Connecting to: %s \n", url->host().data() );

    const socket_factory* sf = socket_factory::instance(ec);
	s_socket tpc_socket = sf->client_tcp_socket(ec, url->host().data(),url->port() );
	io::check_error_code(ec);


	const io::secure::service *sec_service = io::secure::service::instance(ec);
    io::check_error_code( ec );
    io::s_read_write_channel raw_ch = tpc_socket->connect(ec);
    io::check_error_code( ec );
    io::s_read_write_channel sch = sec_service->new_client_connection(ec, std::move(raw_ch) );
    io::check_error_code(ec);

	http::s_request rq = http::new_request( ec, http::method::get, url );

	io::check_error_code( ec );
	rq->send( ec, sch );
	io::check_error_code( ec );

	// 2k
	uint8_t tmp[2048];
	std::size_t read;
	do {
		read = sch->read(ec, tmp, 2048);
		if(read == 0)
			break;
		log( reinterpret_cast<char*>(tmp), read);
	} while(!ec);

    return 0;
}
