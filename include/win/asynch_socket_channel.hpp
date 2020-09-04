#ifndef __ASYNCH_WIN_SOCKET_CHANNEL_HPP_INCLUDED__
#define __ASYNCH_WIN_SOCKET_CHANNEL_HPP_INCLUDED__

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#ifndef SECURITY_WIN32
#	define SECURITY_WIN32
#endif // SECURITY_WIN32

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>

namespace io
{

namespace win {

enum class operation
{
   accept, send, recaive
};

struct overlapped: public ::OVERLAPPED
{
    operation io_op;
    uint8_t *data;
    overlapped(operation op,const ::LARGE_INTEGER& position, uint8_t* px) noexcept:
        ::OVERLAPPED(),
        io_op(op),
        data( px )
    {
        Internal = reinterpret_cast<ULONG_PTR>(nullptr);
        InternalHigh = reinterpret_cast<ULONG_PTR>(nullptr);
        Pointer = nullptr;
        hEvent = static_cast<::HANDLE>(nullptr);
        // set-up offset
        OffsetHigh = position.HighPart;
        Offset = position.LowPart;
    }
};

}  // namespace win

namespace net
{

class IO_PUBLIC_SYMBOL asynch_socket_channel final: public io::asynch_channel
{
public:
    explicit asynch_socket_channel(::SOCKET socket, const asynch_read_completition_routine& rc, const asynch_write_completition_routine& wc) noexcept;
    virtual ~asynch_socket_channel() noexcept override;
    virtual void read(uint8_t* into, std::size_t limit, std::size_t start_from) const noexcept override;
    virtual void write(const uint8_t* what, std::size_t bytes, std::size_t start_from) const noexcept override;
    virtual bool cancel_pending() const noexcept override;
    virtual bool cancel_all() const noexcept override;
private:
	::SOCKET native() const noexcept {
		return reinterpret_cast<::SOCKET>(handle());
	}
};

} // namespace net

} // namespace io

#endif // __ASYNCH_WIN_SOCKET_CHANNEL_HPP_INCLUDED__
