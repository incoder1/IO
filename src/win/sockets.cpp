/*
 *
 * Copyright (c) 2017
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#include "stdafx.hpp"
#include "sockets.hpp"

namespace io {

namespace net {

#ifdef IO_IS_LITTLE_ENDIAN

static inline uint16_t io_ntohs(uint16_t x)
{
    return io_bswap16( x );
}

static inline uint16_t io_htons(uint16_t x)
{
    return io_bswap16( x );
}

#else

static inline uint16_t io_ntohs(uint16_t x)
{
    return x;
}

static inline uint16_t io_htons(uint16_t x)
{
    return x;
}

#endif // IO_IS_LITTLE_ENDIAN


// socket
socket::socket() noexcept:
    object()
{}

// endpoint
endpoint::endpoint(const std::shared_ptr<::addrinfo>& info) noexcept:
    addr_info_(info)
{}

static void addrin_set_port(::PSOCKADDR_IN addrin, uint16_t port) noexcept
{
	 addrin->sin_port = io_htons(port);
}

static void addrin_set_port(::PSOCKADDR_IN6 addrin, uint16_t port) noexcept
{
	 addrin->sin6_port = io_htons(port);
}

static uint16_t addrin_get_port(::PSOCKADDR_IN addrin) noexcept
{
	return io_htons(addrin->sin_port);
}

static uint16_t addrin_get_port(::PSOCKADDR_IN6 addrin) noexcept
{
	 return io_ntohs(addrin->sin6_port);
}


uint16_t endpoint::port() const noexcept
{
    switch( family() ) {
    case ip_family::ip_v4:
        return addrin_get_port( reinterpret_cast<::PSOCKADDR_IN>(addr_info_->ai_addr) );
    case ip_family::ip_v6:
        return addrin_get_port( reinterpret_cast<::PSOCKADDR_IN6>(addr_info_->ai_addr) );
	default:
		io_unreachable
		return 0;
    }
}


void endpoint::set_port(uint16_t port) noexcept
{
    switch( family() ) {
    case ip_family::ip_v4:
        addrin_set_port( reinterpret_cast<::PSOCKADDR_IN>(addr_info_->ai_addr), port);
        break;
    case ip_family::ip_v6:
        addrin_set_port(reinterpret_cast<::PSOCKADDR_IN6>(addr_info_->ai_addr), port);
        break;
    }
}

ip_family endpoint::family() const noexcept
{
    return static_cast<ip_family>( addr_info_->ai_family );
}

const void* endpoint::native() const noexcept
{
    return static_cast<void*>(addr_info_.get());
}

const_string endpoint::ip_address() const noexcept
{
    char tmp[INET6_ADDRSTRLEN];
    io_zerro_mem(tmp, INET6_ADDRSTRLEN);
    const char* ret = ::InetNtopA(
                          addr_info_->ai_family,
                          const_cast<void*>(
                              static_cast<const void*>(addr_info_->ai_addr)
                          ),
                          tmp,
                          INET6_ADDRSTRLEN);
    return (nullptr != ret) ? const_string(tmp) : const_string();
}


static ::SOCKET new_scoket(std::error_code& ec, ip_family af, transport prot) noexcept
{
    int type = 0;
    switch(prot) {
    case transport::tcp:
        type = SOCK_STREAM;
        break;
    case transport::udp:
        type = SOCK_DGRAM;
        break;
    case transport::icmp:
    case transport::icmp6:
        type = SOCK_RAW;
        break;
    }

    ::SOCKET ret = ::WSASocketW(
                       static_cast<int>(af),
                       type,
                       static_cast<int>(prot),
                       nullptr, 0, 0 );
    if(ret == INVALID_SOCKET)
        ec = make_wsa_last_error_code();
    if(ip_family::ip_v6 == af) {
        int off = 0;
        ::setsockopt(
            ret,
            IPPROTO_IPV6,
            IPV6_V6ONLY,
            reinterpret_cast<const char*>(&off),
            sizeof(off)
        );
    }
    return ret;
}

// inet_socket
class inet_socket final: public socket {
public:
    inet_socket(endpoint&& ep, transport t_prot) noexcept:
        socket(),
        transport_(t_prot),
        connected_(false),
        ep_( std::forward<endpoint>(ep) ) {
    }

    virtual endpoint get_endpoint() const noexcept override {
        return ep_;
    }

    virtual transport transport_protocol() const noexcept override {
        return transport_;
    }

    virtual bool connected() const noexcept override {
        return connected_.load( std::memory_order_seq_cst );
    }

    virtual s_read_write_channel connect(std::error_code& ec) const noexcept override {
        bool tmp = connected_.load( std::memory_order_relaxed );
        if( tmp ||
                !connected_.compare_exchange_strong(
                    tmp, true,
                    std::memory_order_acquire,
                    std::memory_order_relaxed
                )
          ) {
            ec = std::make_error_code( std::errc::device_or_resource_busy );
            return s_read_write_channel();
        }
        ::SOCKET s = new_scoket(ec, ep_.family(), transport_);
        if(ec)
            return s_read_write_channel();
        const ::addrinfo *ai = static_cast<const ::addrinfo *>(ep_.native());
        if(
            SOCKET_ERROR == ::WSAConnect(s,
                                         ai->ai_addr, static_cast<int>(ai->ai_addrlen),
                                         nullptr,nullptr,nullptr,nullptr
                                        )
        ) {
            ec = make_wsa_last_error_code();
            connected_.store(false, std::memory_order_release );
            return s_read_write_channel();
        }
        connected_.store(true, std::memory_order_release );
        return s_read_write_channel( nobadalloc<synch_socket_channel>::construct(ec, s ) );
    }
private:
    transport transport_;
    mutable std::atomic_bool connected_;
    endpoint ep_;
};

// socket_factory
std::atomic<socket_factory*> socket_factory::_instance(nullptr);
critical_section socket_factory::_init_cs;

static void initialize_winsocks2(std::error_code& ec) noexcept
{
    ::WSADATA wsadata;
    ::DWORD err = ::WSAStartup( MAKEWORD(2,2), &wsadata );
    if (ERROR_SUCCESS != err) {
        ec = make_wsa_last_error_code();
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
            if(!ec) {
                std::atexit(&socket_factory::do_release);
                ret = nobadalloc<socket_factory>::construct(ec);
                _instance.store(ret, std::memory_order_release);
            }
            _instance.store(ret, std::memory_order_release);
        }
    }
    return ret;
}

static void freeaddrinfo_wrap(void* const p) noexcept
{
    ::freeaddrinfo( static_cast<::addrinfo*>(p) );
}

std::shared_ptr<::addrinfo> socket_factory::get_host_by_name(std::error_code& ec, const char* host) const noexcept
{
    ::addrinfo *ret = nullptr;
    if(0 != ::getaddrinfo(host, nullptr, nullptr, &ret) ) {
        ec = std::make_error_code(std::errc::no_such_device_or_address);
        if(nullptr != ret)
            ::freeaddrinfo(ret);
    }
    return std::shared_ptr<::addrinfo>(ret, freeaddrinfo_wrap );
}

s_socket socket_factory::client_tcp_socket(std::error_code& ec, const char* host, uint16_t port) const noexcept
{
    endpoint ep( get_host_by_name(ec, host) );
    ep.set_port( port );
    if(ec)
        return s_socket();
    return nobadalloc<inet_socket>::construct( ec, std::move(ep), transport::tcp );
}

s_socket socket_factory::client_udp_socket(std::error_code& ec, const char* host, uint16_t port) const noexcept
{
    endpoint ep( get_host_by_name(ec, host) );
    ep.set_port( port );
    if(ec)
        return s_socket();
    return nobadalloc<inet_socket>::construct( ec, std::move(ep), transport::udp );
}



} // namespace net

} // namespace io
