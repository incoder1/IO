/**
* Example demonstrate IO network feature, like secured TLS connection, URL and TCP/IP blocking connection
* for non blocking connection see async_network_client
*/
#include <errorcheck.hpp>

#include <net/uri.hpp>
#include <net/http_client.hpp>
#include <net/security.hpp>

#include <console.hpp>


int main()
{

	using namespace io::net;
	std::error_code ec;
	s_uri url = uri::parse(ec, "https://tools.ietf.org/html/rfc2616");
	io::check_error_code(ec);

	std::ostream& cout = io::console::out_stream();
	io::console::reset_out_color(io::text_color::navy_green);

	cout << "Connecting to: " <<  url->host().data() << std::endl;

	// IO context, entry point to network and asynchronous input oputput
	io::s_io_context ioc = io::io_context::create(ec);
	io::check_error_code(ec);

	// TLS service for HTTPS
	io::net::s_security_context secure_ctx = io::net::security_context::create(ec, ioc);

	// Construct TLS security channe;
    io::s_read_write_channel sch = secure_ctx->client_blocking_connect(ec, url->host().data(), url->port());
    io::check_error_code(ec);

    // Construct writer to communicate with service true HTTP 1.1
    io::writer httpw(sch);

	// Construct new HTTP GET request
	http::s_request rq = http::new_get_request( ec, url );
	io::check_error_code( ec );
	// Send request to HTTP server
	rq->send( ec, httpw );
	io::check_error_code( ec );

	io::console::reset_out_color( io::text_color::white );

	// Read HTTP response from server
	uint8_t buff[1024];
	std::size_t read;
	do {
		io_zerro_mem(buff, sizeof(buff));
		read = sch->read(ec, buff, sizeof(buff) );
		if(read > 0) {
			cout.write(reinterpret_cast<const char*>(buff), read);
			cout.flush();
		}
	} while(0 != read && !ec);

	if(ec) {
		std::ostream& cerr = io::console::error_stream();
		cerr<< "Network error: " << ec.value() << " " << ec.message() << std::endl;
		return ec.value();
	}

    return 0;
}
