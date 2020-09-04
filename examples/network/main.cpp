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

//#define IO_TLS_PROVIDER_GNUTSL

#include <errorcheck.hpp>
#include <network.hpp>

#include <net/uri.hpp>
#include <net/http_client.hpp>
#include <net/secure_channel.hpp>

#include <fstream>
#include <iostream>

#ifdef __IO_WINDOWS_BACKEND__

static void log(const uint8_t* data,const std::size_t bytes)
{
	static ::HANDLE hcout = ::GetStdHandle(STD_OUTPUT_HANDLE);
	::DWORD written;
    ::WriteFile(hcout, data, bytes, &written, nullptr);
}

#else

static void log(const uint8_t* data,const std::size_t bytes)
{
	::fwrite(data, sizeof(uint8_t), bytes, stdout);
}

#endif // __IO_WINDOWS_BACKEND__


int main()
{

	using namespace io::net;
	std::error_code ec;
	s_uri url = uri::parse(ec, "https://www.google.com/");
	io::check_error_code(ec);

	std::printf("Connecting to: %s \n", url->host().data() );

	const io::net::secure::service *cfactory = io::net::secure::service::instance(ec);
    io::check_error_code( ec );

    io::s_read_write_channel sch = cfactory->new_client_blocking_connection(ec, url);
    io::check_error_code(ec);

    io::writer httpw(sch);

	http::s_request rq = http::new_get_request( ec, url );
	io::check_error_code( ec );
	rq->send( ec, httpw );
	io::check_error_code( ec );

	// 2k
	const std::size_t page_size = io::memory_traits::page_size();
	io::scoped_arr<uint8_t> buff(page_size);
	std::size_t read;
	do {
		io_zerro_mem( buff.get(), page_size );
		read = sch->read(ec, buff.get(), page_size - 2 );
		if(read == 0)
			break;
		log( buff.get() , read);
	} while(!ec);
	int ret = 0;
	if(ec) {
		std::cerr<< "Network error no: " << ec.value() << " " << ec.message() << std::endl;
		ret = ec.value();
	}
    return ret;
}
