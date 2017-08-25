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

#include <network.hpp>

#include <net/uri.hpp>
#include <net/http_client.hpp>

#include <errorcheck.hpp>
#include <stream.hpp>
#include <scoped_array.hpp>

int main()
{
	using namespace io::net;
	std::error_code ec;
	s_uri url = uri::parse(ec, "http://www.springframework.org/schema/beans/spring-beans-4.2.xsd");
	//s_uri url = uri::parse(ec, "https://www.springframework.org/beans/spring-beans-4.2.xsd");
	io::check_error_code(ec);
	const socket_factory* sf = socket_factory::instance(ec);
	io::check_error_code(ec);
	std::cout << "connectig to: " << url->host() << std::endl;
	s_socket tpc_socket = sf->client_socket(ec, url->host().data(),url->port() );
	io::check_error_code(ec);
	std::cout << url->host() << "resolved to: \n";
	endpoint ep = tpc_socket->get_endpoint();
	do {
		std::cout << "\t" << ep.ip_address() << '\n';
		ep = ep.next();
	} while( ep.has_next() );
	std::cout << std::endl;
	io::s_read_write_channel sock = tpc_socket->connect(ec);
	io::check_error_code(ec);

	http::s_request rq = http::new_request(
			ec,
			http::method::get,
			url,
			{
				{"Accept","text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8"},
				{"User-Agent", "io library"},
				{"Accept-Charset","ISO-8859-1,utf-8;q=0.7,*;q=0.7"},
				{"Connection"," close"}
			}
	);

	io::check_error_code( ec );
	rq->send( ec, sock );
	io::check_error_code( ec );

	io::scoped_arr<uint8_t> tmp( io::memory_traits::page_size() << 3 );
	if(!tmp)
		io::check_error_code( std::make_error_code(std::errc::not_enough_memory) );

	std::size_t read;
	do {
		read = sock->read(ec, tmp.get(), tmp.len() );
		if(read > 0)
			std::cout.write( reinterpret_cast<const char*>(tmp.get()), read);
	} while(!ec && read > 0 );

    return 0;
}
