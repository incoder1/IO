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
		::WSABUF wsab;
		wsab.len = static_cast<::u_long>(bytes);
		wsab.buf = const_cast<char*>(reinterpret_cast<const char*>(buff));
		::DWORD ret, flags = 0;
		if(SOCKET_ERROR == ::WSARecv(socket_, &wsab, 1, &ret, &flags, nullptr, nullptr) ) {
			ec.assign( ::WSAGetLastError(), std::system_category() );
			return 0;
		}
		return static_cast<::std::size_t>(ret);
	}
	virtual std::size_t write(std::error_code& ec, const uint8_t* buff,std::size_t size) const noexcept override
	{
		::WSABUF wsab;
		wsab.len = static_cast<::u_long>(size);
		wsab.buf = const_cast<char*>(reinterpret_cast<const char*>(buff));
		::DWORD ret;
		if(SOCKET_ERROR == ::WSASend(socket_, &wsab, 1, &ret, 0, nullptr, nullptr) ) {
			ec.assign( ::WSAGetLastError(), std::system_category() );
			return 0;
		}
		return static_cast<::std::size_t>(ret);
	}
public:
	::SOCKET socket_;
};

// socket
socket::socket() noexcept:
	object()
{}

// endpoint
endpoint::endpoint(std::shared_ptr<::addrinfo>&& info) noexcept:
	addr_info_(std::forward<std::shared_ptr<::addrinfo> >(info))
{
}

endpoint::endpoint(const std::shared_ptr<::addrinfo>& info) noexcept:
	addr_info_(info)
{}


uint16_t endpoint::port() const noexcept
{
	return ::ntohs( native()->sin_port );
}

void endpoint::set_port(uint16_t port) noexcept
{
	::sockaddr_in *addr = const_cast<::sockaddr_in*>(native());
	addr->sin_port = port;
}

ip_family endpoint::family() const noexcept
{
	return static_cast<ip_family>( addr_info_->ai_family );
}

const ::sockaddr_in* endpoint::native() const noexcept
{
	return reinterpret_cast<::sockaddr_in*>( addr_info_->ai_addr );
}

const_string endpoint::ip_address() const noexcept
{
	char tmp[INET6_ADDRSTRLEN];
	io_zerro_mem(tmp, INET6_ADDRSTRLEN);
	const char* ret = ::InetNtop(native()->sin_family,
	                             const_cast<void*>(
	                                 static_cast<const void*>(&(native()->sin_addr))
	                             ),
	                             tmp, INET6_ADDRSTRLEN);
	return nullptr != ret ? const_string(tmp) : const_string() ;
}


class tpc_socket final: public socket {
public:
	tpc_socket(endpoint&& ep) noexcept:
		socket(),
		ep_( std::forward<endpoint>(ep) )
	{}
	virtual endpoint get_endpoint() const noexcept
	{
		return ep_;
	}
	virtual s_read_write_channel connect(std::error_code& ec) const noexcept override
	{
		::SOCKET s = ::WSASocketA( static_cast<int>(
		                           ep_.family()
		                       ), SOCK_STREAM, IPPROTO_TCP, nullptr, 0, 0 );
		if(s == INVALID_SOCKET) {
			ec.assign( ::WSAGetLastError(), std::system_category() );
			return s_read_write_channel();
		}
		if(SOCKET_ERROR == ::connect(s,
								reinterpret_cast<const ::sockaddr*>(ep_.native()),
								sizeof(::SOCKADDR_IN) ) )
		{
			// TODO: change
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
	endpoint ep_;
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

static void freeaddrinfo_wrap(void* const p) noexcept {
	::freeaddrinfo( static_cast<::addrinfo*>(p) );
}

static s_socket creatate_tcp_socket(std::error_code& ec, ::addrinfo *addr, uint16_t port) noexcept {
	endpoint ep( std::shared_ptr<::addrinfo>(addr, freeaddrinfo_wrap ) );
	ep.set_port( ::htons(port) );
	return s_socket( nobadalloc<tpc_socket>::construct(ec, std::move(ep) ) );
}

s_socket socket_factory::client_socket(std::error_code& ec, const char* host, uint16_t port) const noexcept
{
	::addrinfo *addr = nullptr;
	int err = ::getaddrinfo(host, nullptr, nullptr, &addr);
	// TODO: should be custom error code
	if(0 != err) {
		ec = std::make_error_code(std::errc::no_such_device_or_address);
		if(nullptr != addr)
			::freeaddrinfo(addr);
		return s_socket();
	}
	switch(addr[0].ai_family) {
	case AF_INET:
	case AF_INET6:
		return creatate_tcp_socket(ec, addr, port);
	default:
		if(nullptr != addr)
			::freeaddrinfo(addr);
		ec = std::make_error_code(std::errc::operation_not_permitted);
	}
	return s_socket();
}

} // namespace net

} // namespace io
