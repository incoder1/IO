#include "stdafx.hpp"
#include "tcp_ip.hpp"

#ifndef IO_HAS_BOOST
#	error "io network channels requares boost assio please build with boost"
#endif // IO_HAS_BOOST

#include <boost/array.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/address_v4.hpp>
#include <boost/asio/ip/address_v6.hpp>
#include <boost/asio/connect.hpp>

#include <boost/asio/io_service.hpp>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/udp.hpp>

#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <boost/lexical_cast.hpp>

#include "error_category_shim.hpp"

namespace asio = boost::asio;

namespace io {

// synchronius socket channel
template<class __socket>
class sync_socket_channel: public read_write_channel
{
public:
	typedef __socket socket_type;
	sync_socket_channel(socket_type&& socket) noexcept:
		read_write_channel(),
		socket_( std::forward<socket_type>(socket) )
	{}
	virtual ~sync_socket_channel() noexcept override
	{
		boost::system::error_code ec;
        socket_.close( ec );
        if(ec)
			std::unexpected();
	}
	virtual std::size_t read(std::error_code& ec,uint8_t* const buff, std::size_t bytes) const noexcept override
	{
		std::size_t ret;
		boost::system::error_code errcode;
		ret = asio::read(socket_, asio::buffer( asio::mutable_buffer(buff,bytes) ), errcode);
		if(errcode)
			convert_error_codes(ec,errcode);
		return ret;
	}
	virtual std::size_t write(std::error_code& ec, const uint8_t* buff,std::size_t size) const noexcept override
	{
		std::size_t ret;
		boost::system::error_code errcode;
		ret = asio::write(socket_, asio::buffer( static_cast<const void*>(buff), size), errcode);
		if(errcode)
			convert_error_codes(ec,errcode);
		return ret;
	}
public:
	mutable socket_type socket_;
};

typedef asio::ip::tcp::socket tcp_socket;

class sync_tcp_socket_channel final: public sync_socket_channel<tcp_socket>
{
public:
	sync_tcp_socket_channel(tcp_socket&& socket) noexcept:
		sync_socket_channel<tcp_socket>( std::forward<tcp_socket>(socket) )
	{}
};

class network_factory_impl final: public network_factory
{
public:
	network_factory_impl() noexcept;
	virtual ~network_factory_impl() noexcept;
	virtual s_read_write_channel tcp_sync_channel(std::error_code& ec, const network_address address) const noexcept override;
private:
	mutable asio::io_service io_service_;
};

const network_factory* IO_PUBLIC_SYMBOL get_network_factory(std::error_code& ec) noexcept
{
	network_factory_impl *ret = nobadalloc<network_factory_impl>::construct(ec);
	return ret;
}

network_factory_impl::network_factory_impl() noexcept:
	network_factory(),
	io_service_()
{
}

network_factory_impl::~network_factory_impl() noexcept
{}

s_read_write_channel network_factory_impl::tcp_sync_channel(std::error_code& ec, const network_address address) const noexcept
{
	tcp_socket socket( io_service_ );
	asio::ip::tcp::resolver resolver(io_service_);
	boost::system::error_code errcode;
	std::string h(address.hostname);
	std::string s = boost::lexical_cast<std::string>(address.port);
	asio::ip::tcp::resolver::query q( asio::ip::tcp::v4(), h, s );
	boost::asio::connect(socket, resolver.resolve(q), errcode );
	if(errcode) {
		convert_error_codes( ec, errcode);
		return s_read_write_channel();
	}
	sync_tcp_socket_channel *ch = nobadalloc<sync_tcp_socket_channel>::construct(ec, std::move(socket) );
	if(ec)
		return s_read_write_channel();
	return s_read_write_channel(ch);
}


} // namespace io
