#ifndef __IO_WIN_OVERLAPPED_HPP__
#define __IO_WIN_OVERLAPPED_HPP__

#include "config.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include "buffer.hpp"

namespace io {

namespace detail {

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

}  // namespace detail

} // namespace IO

#endif // __IO_WIN_OVERLAPPED_HPP__
