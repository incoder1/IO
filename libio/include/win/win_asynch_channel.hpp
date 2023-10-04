#ifndef __IO_WIN_ASYNC_CHANNEL_HPP__
#define __IO_WIN_ASYNC_CHANNEL_HPP__

#include "config.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include "buffer.hpp"
#include "channels.hpp"

namespace io {

namespace win {

enum class operation: ::DWORD {
   accept = 0,
   send = 1,
   recaive = 2
};

struct overlapped: public ::OVERLAPPED {
    overlapped(operation op, byte_buffer&& data, uint64_t position) noexcept:
    	OVERLAPPED(),
		io_op_(op),
		data_( std::forward<byte_buffer>(data) )
    {
        Internal = reinterpret_cast<ULONG_PTR>(nullptr);
        InternalHigh = reinterpret_cast<ULONG_PTR>(nullptr);
        // set-up offset
		::LARGE_INTEGER pos;
		pos.QuadPart = position;
        OffsetHigh = pos.HighPart;
        Offset = pos.LowPart;
        hEvent = static_cast<::HANDLE>(nullptr);
    }
	operation io_op_;
    byte_buffer data_;
};


class win_asynch_channel:public asynch_channel
{
	win_asynch_channel(const win_asynch_channel&) = delete;
	win_asynch_channel& operator=(const win_asynch_channel&) = delete;
public:
	win_asynch_channel(::HANDLE hnd,const s_asynch_completion_routine& routine, const asynch_io_context* context) noexcept:
		asynch_channel(routine,context),
		hnd_(hnd)
	{}
	::HANDLE handle() const noexcept {
		return hnd_;
	};
private:
	::HANDLE hnd_;
};


}  // namespace win

} // namespace IO

#endif // __IO_WIN_ASYNC_CHANNEL_HPP__
