/*
 *
 * Copyright (c) 2016-2020
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

	constexpr explicit handle_channel(::HANDLE hnd) noexcept:
		hnd_(hnd)
	{}

	~handle_channel() noexcept
	{
		::CloseHandle(hnd_);
	}

    std::size_t read(std::error_code& err, uint8_t* const buff, std::size_t bytes) const noexcept
	{
		::DWORD result;
		if( ! ::ReadFile(hnd_, void_cast(buff), static_cast<DWORD>(bytes), &result, nullptr) )
			err.assign(::GetLastError(), std::system_category() );
		return result;
	}

	std::size_t write(std::error_code& err, const uint8_t* buff,std::size_t size) const noexcept
	{
		::DWORD result;
		if ( ! ::WriteFile(hnd_, void_cast(buff), static_cast<::DWORD>(size), &result, nullptr) )
			err.assign(::GetLastError(), std::system_category() );
		return result;
	}


#ifdef IO_CPU_BITS_64
	int64_t seek(std::error_code& err, whence_type whence, int64_t dist) noexcept
	{
		::LARGE_INTEGER li;
		li.QuadPart = dist;
		if( !::SetFilePointerEx(hnd_, li, &li, static_cast<::DWORD>(whence) ) ) {
			err.assign(::GetLastError(), std::system_category() );
		}
		return li.QuadPart;
	}
#else
	long seek(std::error_code& err, whence_type whence, long dist) noexcept
	{
		::LARGE_INTEGER li;
		li.QuadPart = dist;
		if( !::SetFilePointerEx(hnd_, li, &li, static_cast<::DWORD>(whence) ) ) {
			err.assign(::GetLastError(), std::system_category() );
		}
		return li.QuadPart;
	}
#endif // IO_CPU_BITS_64

	operator HANDLE () const noexcept
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
