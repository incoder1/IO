/**
* Example demonstrate IO blocking TCP/IP and HTTP network client input-output
* including secured TLS protected HTTP connection.
* For non blocking connection see async_network_client
*/
#include <errorcheck.hpp>

#include <net/uri.hpp>
#include <net/http_client.hpp>
#include <net/security.hpp>
#include <iostream>

int main(int argc, const char** argv)
{

	using namespace io::net;
	std::error_code ec;
	s_uri url = uri::parse(ec, "https://www.rfc-editor.org/rfc/rfc9110.html");
	io::check_error_code(ec);


    std::ios::sync_with_stdio(false);
	std::cout << "Connecting to: " <<  url->host().data() << std::endl;

	// IO context, entry point to network and asynchronous input oputput
	io::s_io_context ioc = io::io_context::create(ec);
	io::check_error_code(ec);

	// TLS service for HTTPS
	io::net::tls::s_security_context secure_ctx = io::net::tls::security_context::create(ec, ioc);
	io::check_error_code(ec);

	// Construct TLS secured channel;
    io::s_read_write_channel sch = secure_ctx->client_blocking_connect(ec, url->host().data(), url->port());
    io::check_error_code(ec);

    // Construct reader and writer to communicate with service over HTTP 1.1
    io::reader httpr(sch);
    io::writer httpw(sch);

	// Construct new HTTP GET requests
	http::s_request rq = http::new_get_request( ec, url );
	io::check_error_code( ec );

	// Send request to HTTP server
	rq->send( ec, httpw );
	io::check_error_code( ec );

	// Read HTTP response from server
    io::scoped_arr<char> buff( io::memory_traits::page_size() * 2 );
	std::size_t read;
	do {
		read = httpr.read(ec, buff.begin(), buff.len() );
		if(read > 0) {
			std::cout.write(buff.begin(), read);
			buff[0] = '\0';
		}
	} while( !ec && read > 0 );
	std::cout.flush();

	if(ec) {
		std::cerr << "Network error: " << ec.value() << " " << ec.message() << std::endl;
		return ec.value();
	}

    return 0;
}
