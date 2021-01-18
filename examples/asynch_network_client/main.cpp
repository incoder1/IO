/**
* Example demonstrate IO network feature, non blocking asynronius TCP/IP connection
* for blocking connection see network
*/
#include <errorcheck.hpp>
#include <network.hpp>
#include <iostream>

/// Asynchronous IO operations completion routine
class my_routine final: public io::asynch_completion_routine {
private:
    explicit my_routine() noexcept:
        io::asynch_completion_routine()
    {}
public:
    /// Creates new routine reference on implementation smart pointer
    /// ec contains operation error code, like out of memory
    static io::s_asynch_completion_routine create(std::error_code& ec) noexcept;
    /// Handles asynchronous data received i.e. read is done
    /// \param ec
    ///             operation error code like network is down etc.
    /// \param source
    ///             a async_channel which was used to asynchronously receive data
    /// \param data
    ///             received data buffer, data.length() shows transmit data, buffer position is set to head when
	///             last iterator shows on a byte after last received byte
    virtual void received(std::error_code& ec, const io::s_asynch_channel& source, io::byte_buffer&& data) noexcept override;
    /// Handles asynchronous data sent i.e. write is done
	/// \param ec
    ///             operation error code like network is down etc.
    /// \param source
    ///             a async_channel which was used to asynchronously send data
    /// \param data
    ///             sent data buffer, position iterator shows on last sent byte, what.lenght() can be used to detect not sent tail bytes
    virtual void sent(std::error_code& ec,const io::s_asynch_channel& source, io::byte_buffer&& what) noexcept override;
};

io::s_asynch_completion_routine my_routine::create(std::error_code& ec) noexcept
{
    if(!ec) {
        my_routine *ret = new (std::nothrow) my_routine();
        if(nullptr == ret)
            ec = std::make_error_code(std::errc::not_enough_memory);
        else
            return io::s_asynch_completion_routine(ret);
    }
    return io::s_asynch_completion_routine();
}

void my_routine::received(std::error_code& ec, const io::s_asynch_channel& source, io::byte_buffer&& data) noexcept
{
    if(!ec) {
        std::cout <<  data.length() << " received from server\n";
        std::cout.write(data.position().cdata(), data.length() );
        std::cout.flush();
    }
    // notify main application thread, waiting for pending IO operations, we've done
    // and shutdown IO completion thread pool
	source->context()->shutdown();
	if(ec)
		std::cerr << "Error: " << ec.value() << ' ' << ec.message() << std::endl;
}

void my_routine::sent(std::error_code& ec,const io::s_asynch_channel& source, io::byte_buffer&& what) noexcept
{
    if(!ec) {
		// check all data sent
		if(what.empty()) {
			what.flip();
			std::cout<< "HTTP 1.1 Request sent: \n" <<  what.position().cdata() << std::endl;
			// now asynchronously read HTTP response from server
			if(!ec)
				source->recaive(ec, io::memory_traits::page_size(), 0);
		} else {
			// some data left, send the tail part
			// since we've using socket, position ramainig 0
			source->send(ec, std::forward<io::byte_buffer>(what), 0);
		}
    }
    if(ec) {
		std::cerr << "Error: " << ec.value() << ' ' << ec.message() << std::endl;
		// in case of error, shutdown asynch io completion handling,
		// and release threads waiting for it by io::s_io_context::await_asynchronous
		// in our case it is our main application thread
        source->context()->shutdown();
    }
}

// Simple HTTP 1.1 Request to a server, for demonstration propose
// HTTP client, used to for really tinny communicate with
// a public available server, in our case it is W3C
const char* HTTP_GET_REQUEST = "GET /html/rfc2616 HTTP/1.1\r\n\
Host: tools.ietf.org\r\n\
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n\
Cache-Control: no-cache\r\n\
Pragma: no-cache\r\n\
Connection: close\r\n\
Accept-Language: en-US,en;q=0.5\r\n\
User-Agent: IO/2020  (C++ HTTP client lib)\r\n\r\n";

int main(int argc, const char** argv)
{
    std::error_code ec;

    // Create a network socket to connect
    const io::net::socket_factory *sf = io::net::socket_factory::instance(ec);
    io::check_error_code(ec);
    io::s_asynch_io_context aioc = io::asynch_io_context::create(ec);
    io::check_error_code(ec);

    // We'll request HTTP 1.1. RFC document from W3C
    // this example shows work with raw network sockets with asynchronous IO,
    // and generally not about HTTP protocol it self
    // Real HTTP communication need TLS security layer to support communication over HTTPS
    io::net::socket socket = sf->client_tcp_socket(ec, "tools.ietf.org", 80);
    io::check_error_code(ec);

	//Create asynchronous completion routine object instance
	// which will handle all asynchronous operations complete states
	io::s_asynch_completion_routine routine = my_routine::create(ec);
    io::check_error_code(ec);
    // Connect to the server using asynchronous input/ouput
    io::s_asynch_channel asch = aioc->client_asynch_connect(ec, std::move(socket), routine );

    // Wrap HTTP request text into byte buffer and send HTTP request asynchronously
    io::byte_buffer buff = io::byte_buffer::wrap(ec, HTTP_GET_REQUEST );
    io::check_error_code(ec);
    asch->send(ec, std::move(buff), 0);
    io::check_error_code(ec);

    // Await for all pending asynchronous operations to be done.
    // Don't allow main CRT thread to finish and terminate
    // all threads with in completion thread pool
    // so that our IO operations would't be lost
    aioc->await();

    return 0;
}
