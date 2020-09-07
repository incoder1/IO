/**
* Example demonstrate IO network feature, non blocking asynronius TCP/IP connection
* for blocking connection see network
*/
#include <errorcheck.hpp>
#include <network.hpp>
#include <iostream>

class my_routine final: public io::asynch_completion_routine {
private:
    explicit my_routine() noexcept:
        io::asynch_completion_routine()
    {}
public:
    static io::s_asynch_completion_routine create(std::error_code& ec) noexcept
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
    virtual void recaived(std::error_code& ec, const io::s_asynch_channel& source, const uint8_t* data, std::size_t transferred) noexcept override
    {
        if(!ec) {
            std::cout.write(reinterpret_cast<const char*>(data), transferred);
            std::cout.flush();
            io::memory_traits::free_temporary( const_cast<uint8_t*>(data) );
        }
        source->context()->shutdown_asynchronous(ec);
        if(ec)
            std::cerr << ec.message() << std::endl;
    }
    virtual void sent(std::error_code& ec,const io::s_asynch_channel& source, const uint8_t* data, std::size_t transfered) noexcept override
    {
        if(!ec) {
			std::cout<< "HTTP 1.1 Request sent: \n" <<  reinterpret_cast<const char*>(data) << std::endl;
            // now asynchroniusly read HTTP response from server
            source->read(io::memory_traits::calloc_temporary<uint8_t>(4096) , 4096, 0);
        }
        if(ec) {
            source->context()->shutdown_asynchronous(ec);
            std::cerr << ec.message() << std::endl;
        }
    }
};

/// Simple HTTP 1.1 Request to a server
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
    const io::net::socket_factory *sf = io::net::socket_factory::instance(ec);
    io::check_error_code(ec);
    io::s_io_context ioc = io::io_context::create(ec);
    io::check_error_code(ec);
    io::net::socket socket = sf->client_tcp_socket(ec, "tools.ietf.org", 80);
    io::check_error_code(ec);

    io::s_asynch_completion_routine routine = my_routine::create(ec);
    io::check_error_code(ec);

    io::s_asynch_channel asch = ioc->client_asynch_connect(ec, std::move(socket), routine );
    // Send HTTP request
    asch->write( reinterpret_cast<const uint8_t*>(HTTP_GET_REQUEST), std::strlen(HTTP_GET_REQUEST), 0);
    ioc->await_asynchronous(ec);
    io::check_error_code(ec);

    return 0;
}
