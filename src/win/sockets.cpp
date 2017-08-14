#include "../stdafx.hpp"
#include "sockets.hpp"

namespace io {

namespace net {

class synch_socket_channel final:public read_write_channel {
private:
	friend class nobadalloc<synch_socket_channel>;
	synch_socket_channel(::SOCKET socket) noexcept:
		read_write_channel(),
		socket_(socket)
	{}
public:
	virtual ~synch_socket_channel() noexcept
	{
		::closesocket(socket_);
	}
	virtual std::size_t read(std::error_code& ec,uint8_t* const buff, std::size_t bytes) const noexcept override
	{
		return 0;
	}
	virtual std::size_t write(std::error_code& ec, const uint8_t* buff,std::size_t size) const noexcept override
	{
		return 0;
	}
public:
	::SOCKET socket_;
};

// socket_address
socket::socket() noexcept:
	object()
{}


class tpc_socket final: public socket {
public:
	tpc_socket(const ::addrinfo* addr, uint16_t port) noexcept:
		socket(),
		addr_(),
		port_(port)
	{
		io_memmove(&addr_, addr->ai_addr, sizeof(::sockaddr) );
		addr_.sin_port = port_;
	}
	virtual socket_type type() const noexcept
	{
		return socket_type::TCP;
	}
	virtual s_read_write_channel connect(std::error_code& ec) const noexcept override
	{
		::SOCKET s = ::socket(addr_.sin_family, SOCK_STREAM, IPPROTO_TCP);
		if(s == INVALID_SOCKET) {
			ec.assign( ::WSAGetLastError(), std::system_category() );
			return s_read_write_channel();
		}
		if(SOCKET_ERROR == ::connect(s, reinterpret_cast<const ::sockaddr*>(&addr_), sizeof(::SOCKADDR_IN) ) ) {
			int errc = ::WSAGetLastError();
			switch(errc) {
			case WSAEACCES:
				ec = std::make_error_code( std::errc::permission_denied );
				break;
			case WSAEWOULDBLOCK:
				ec = std::make_error_code( std::errc::resource_unavailable_try_again);
				break;
			case WSAEOPNOTSUPP:
				ec = std::make_error_code( std::errc::operation_not_permitted );
				break;
			case WSAEINVAL:
				ec = std::make_error_code( std::errc::invalid_argument );
				break;
			case WSAECONNREFUSED:
			case WSAETIMEDOUT:
				ec = std::make_error_code( std::errc::io_error );
				break;
			}
			return s_read_write_channel();
		}
		return s_read_write_channel( nobadalloc<synch_socket_channel>::construct(ec, s ) );
	}
private:
	::SOCKADDR_IN addr_;

	uint16_t port_;
};

// socket_factory
std::atomic<socket_factory*> socket_factory::_instance(nullptr);
critical_section socket_factory::_init_cs;

static void initialize_winsocks2(std::error_code& ec) noexcept
{
	::WSADATA wsadata;
	::DWORD err = ::WSAStartup( MAKEWORD(2,2), &wsadata );
	if (err != 0) {
		ec.assign( ::WSAGetLastError(), std::system_category() );
		::WSACleanup();
	}
}

void socket_factory::do_release() noexcept
{
	socket_factory *iosrv = _instance.load(std::memory_order_acquire);
	if(nullptr != iosrv)
		delete iosrv;
	_instance.store(nullptr, std::memory_order_release);
}

socket_factory::~socket_factory() noexcept
{
	::WSACleanup();
}

const socket_factory* socket_factory::instance(std::error_code& ec) noexcept
{
	socket_factory *ret = _instance.load(std::memory_order_relaxed);
	if(nullptr == ret) {
		lock_guard lock(_init_cs);
		ret = _instance.load(std::memory_order_acquire);
		if(nullptr == ret) {
			initialize_winsocks2(ec);
			if(ec) {
				_instance.store(ret, std::memory_order_release);
				return nullptr;
			}
			std::atexit(&socket_factory::do_release);
			ret = nobadalloc<socket_factory>::construct(ec);
			if(ec) {
				_instance.store(ret, std::memory_order_release);
				return nullptr;
			}
			_instance.store(ret, std::memory_order_release);
		}
	}
	return ret;
}

s_socket socket_factory::client_socket(std::error_code& ec, const char* host, uint16_t port) const noexcept
{
	::addrinfo *addr = nullptr;
	int err = ::getaddrinfo(host, nullptr, nullptr, &addr);
	s_socket ret;
	// TODO: should be custom error code
	if(0 == err) {
		switch(addr[0].ai_family) {
		case AF_INET:
		case AF_INET6:
			ret.reset( nobadalloc<tpc_socket>::construct(ec, &addr[0], port ) );
			break;
		default:
			::freeaddrinfo(addr);
			ec = std::make_error_code(std::errc::operation_not_permitted);
		}
	} else {
		ec = std::make_error_code(std::errc::no_such_device_or_address);
	}
	if(nullptr != addr)
		::freeaddrinfo(addr);
	return ret;
}

} // namespace net

} // namespace io
