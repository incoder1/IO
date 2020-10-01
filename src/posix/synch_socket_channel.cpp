#include "stdafx.hpp"
#include "posix/synch_socket_channel.hpp"

namespace io {

namespace net {

// synch_socket_channel

synch_socket_channel::synch_socket_channel(int socket) noexcept:
    read_write_channel(),
    socket_(socket)
{}

synch_socket_channel::~synch_socket_channel() noexcept
{
    ::close(socket_);
}

std::size_t synch_socket_channel::read(std::error_code& ec,uint8_t* const buff, std::size_t bytes) const noexcept override
{
    ::ssize_t ret = ::recv(socket_, static_cast<void*>(buff), bytes, 0);
    if(SOCKET_ERROR == ret) {
        ec.assign( errno, std::system_category() );
        return 0;
    }
    return static_cast<::std::size_t>(ret);
}

std::size_t synch_socket_channel::write(std::error_code& ec, const uint8_t* buff,std::size_t size) const noexcept override
{
    ::ssize_t ret = ::send(socket_, static_cast<const void*>(buff), size,  0);
    if(SOCKET_ERROR ==  ret ) {
        ec.assign( errno, std::system_category() );
        return 0;
    }
    return static_cast<::std::size_t>(ret);
}

} // namespace net

} // namespace io

