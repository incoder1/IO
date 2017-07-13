/*
 *
 * Copyright (c) 2016
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef _IO_WIN_HANDLECHANNEL_HPP_INCLUDED__
#define _IO_WIN_HANDLECHANNEL_HPP_INCLUDED__

#include <config.hpp>

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

namespace io {

namespace win {

namespace detail {

constexpr void* void_cast(const uint8_t* ptr)
{
	return const_cast<void*>( static_cast<const void*>(ptr) );
}

enum class whence_type: ::DWORD {
	current = FILE_CURRENT,
	begin = FILE_BEGIN,
	end = FILE_END
};

class handle_channel {
	handle_channel(const handle_channel&) = delete;
	handle_channel& operator=(handle_channel&) = delete;
public:

	constexpr handle_channel(::HANDLE hnd) noexcept:
		hnd_(hnd)
	{}

	~handle_channel() noexcept
	{
		::CloseHandle(hnd_);
	}

	inline std::size_t read(std::error_code& err, uint8_t* const buff, std::size_t bytes) const noexcept
	{
		::DWORD result;
		if( ! ::ReadFile(hnd_, void_cast(buff), bytes, &result, nullptr) )
			err.assign(::GetLastError(), std::system_category() );
		return result;
	}

	inline void asynch_read(::LPOVERLAPPED ovrlp, uint8_t* const to, std::size_t bytes) const noexcept
	{
		 std::error_code ec;
		::BOOL errorCode = ::ReadFile(
								hnd_,
								void_cast( to ),
								bytes,
								NULL, ovrlp);
		::DWORD lastError = ::GetLastError();
		if( !errorCode && ERROR_IO_PENDING != lastError )
			ec.assign( lastError, std::system_category() );
	}

	inline std::size_t write(std::error_code& err, const uint8_t* buff,std::size_t size) const noexcept
	{
		::DWORD result;
		if ( ! ::WriteFile(hnd_, void_cast(buff), size, &result, nullptr) )
			err.assign(::GetLastError(), std::system_category() );
		return result;
	}

	inline void asynch_write(::LPOVERLAPPED ovrlp,const uint8_t* what, std::size_t len) const noexcept
	{
		 std::error_code ec;
		::BOOL errorCode = ::WriteFile(
								hnd_,
								void_cast( what ),
								len,
								NULL, ovrlp);
		::DWORD lastError = ::GetLastError();
		if( !errorCode && ERROR_IO_PENDING != lastError )
			ec.assign( lastError, std::system_category() );
	}


	inline uint64_t seek(std::error_code err,whence_type whence,int64_t offset)
	{
		::LARGE_INTEGER li;
		li.QuadPart = offset;
		if( !::SetFilePointerEx(hnd_, li, &li, static_cast<::DWORD>(whence) ) )
			err.assign(::GetLastError(), std::system_category() );
		return li.QuadPart;
	}

	inline ::HANDLE hnd() const noexcept
	{
		return hnd_;
	}

private:
	::HANDLE hnd_;
};

} // namespace detail

} // namespace win

} // namespace io

#endif // _IO_WIN_HANDLECHANNEL_HPP_INCLUDED__
