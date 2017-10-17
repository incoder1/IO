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

#include <iostream>
#include <files.hpp>

#include <net/uri.hpp>
#include <net/http_client.hpp>
#include <net/secure_channel.hpp>

#include <errorcheck.hpp>
#include <stream.hpp>
#include <scoped_array.hpp>

int main()
{
	using namespace io::net;
	std::error_code ec;
	//s_uri url = uri::parse(ec, "http://www.springframework.org/schema/beans/spring-beans-4.2.xsd");
	s_uri url = uri::parse(ec, "https://www.springframework.org/beans/spring-beans-4.2.xsd");
	io::check_error_code(ec);

	io::check_error_code(ec);
	std::cout << "connectig to: " << url->host() << std::endl;

    const socket_factory* sf = socket_factory::instance(ec);
	s_socket tpc_socket = sf->client_tcp_socket(ec, url->host().data(),url->port() );
	io::check_error_code(ec);


	const io::net::secure::service *sec_service = io::net::secure::service::instance(ec);
    io::check_error_code( ec );

    io::s_read_write_channel raw_ch = tpc_socket->connect(ec);
    io::check_error_code( ec );

    io::s_read_write_channel sch = sec_service->new_client_connection(ec, raw_ch );
    io::check_error_code(ec);

	http::s_request rq = http::new_request( ec, http::method::get, url );

	io::check_error_code( ec );
	rq->send( ec, sch );
	io::check_error_code( ec );

	io::scoped_arr<uint8_t> tmp( 1 << 20);
	if(!tmp)
		io::check_error_code( std::make_error_code(std::errc::not_enough_memory) );

	std::size_t read;
	do {
		read = sch->read(ec, tmp.get(), tmp.len() );
		if(read > 0)
			std::cout.write( reinterpret_cast<const char*>(tmp.get()), read);
	} while(!ec && read > 0 );

    return 0;
}
