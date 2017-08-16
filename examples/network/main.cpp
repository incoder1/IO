#if defined(_WIN32) || defined(_WIN64)

#if _WIN32_WINNT < _WIN32_WINNT_VISTA
#	undef _WIN32_WINNT
#	define _WIN32_WINNT _WIN32_WINNT_VISTA
#else
#	define _WIN32_WINNT _WIN32_WINNT_VISTA
#endif // _WIN32_WINNT

#endif // _WIN32

#include <iostream>

#include <network.hpp>
#include <net/http_client.hpp>


#include <errorcheck.hpp>
#include <stream.hpp>
#include <scoped_array.hpp>
#include <sstream>

int main()
{
	using namespace io::net;
	std::error_code ec;
	const socket_factory* sf = socket_factory::instance(ec);
	io::check_error_code(ec);
	s_socket tpc_socket = sf->client_socket(ec, "boost.org", 80);
	io::check_error_code(ec);
	std::cout << "boost.org resolved to: \n";
	endpoint ep = tpc_socket->get_endpoint();
	do {
		std::cout << "\t" << ep.ip_address();
		ep = ep.next();
	} while( ep.has_next() );
	std::cout << std::endl;
	io::s_read_write_channel sock = tpc_socket->connect(ec);
	io::check_error_code(ec);

	io::s_string_pool sp = io::string_pool::create(ec);
	io::check_error_code(ec);

	http::request<http::method::get> getreq( sp->get("/") , sp->get("www.boost.org") );
	getreq.add_headers( {
			http::default_headers::ACCEPT_HTML_AND_XML,
			http::default_headers::ACCEPT_CHARSET,
			http::default_headers::IO_USER_AGNET,
			std::make_pair("Connection"," close")
		}  );

	io::byte_buffer buff = io::byte_buffer::allocate(ec, io::memory_traits::page_size() );
	io::check_error_code(ec);

	getreq.to_buff(buff);
	buff.flip();

	std::cout << buff.position().cdata();

	sock->write( ec, buff.position().get(), buff.length() );
	io::check_error_code( ec );

	io::scoped_arr<uint8_t> tmp( 1 << 20 ); // 1mb
	if(!tmp)
		io::check_error_code( std::make_error_code(std::errc::not_enough_memory) );

	std::size_t read;
	while(!ec){
		read = sock->read(ec, tmp.get(), tmp.len() );
		if(read > 0)
			std::cout.write( reinterpret_cast<const char*>(tmp.get()), read);
		else
			break;
	}

    return 0;
}
