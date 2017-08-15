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
#include <errorcheck.hpp>
#include <stream.hpp>
#include <scoped_array.hpp>

const char* GET = "GET / HTTP/1.1\r\n";
const char* HEADERS[] = {
	"Host: www.boost.org\r\n",
	"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n",
	"User-Agent:io library\r\n",
	"Accept-Charset:ISO-8859-1,utf-8;q=0.7,*;q=0.7\r\n",
	"Connection: close\r\n\r\n"
};

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

	io::byte_buffer request = io::byte_buffer::allocate(io::memory_traits::page_size());
	request.put(GET);
	for(int i=0; i < 5; i++) {
		request.put( HEADERS[i] );
	}
	request.flip();

	io::unsafe<io::s_read_write_channel> stch( std::move(sock) );
	stch.write(request.position().get(), request.size() );

	io::scoped_arr<uint8_t> tmp( 1 << 20 ); // 1mb
	if(!tmp)
		io::check_error_code( std::make_error_code(std::errc::not_enough_memory) );

	std::size_t read;
	for(;;){
		read = stch.read( tmp.get(), tmp.len() );
		if(read > 0)
			std::cout.write( reinterpret_cast<const char*>(tmp.get()), read);
		else
			break;
	}

    return 0;
}
