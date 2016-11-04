#ifndef _IO_WIN_HANDLECHANNEL_HPP_INCLUDED__
#define _IO_WIN_HANDLECHANNEL_HPP_INCLUDED__

#include <config.hpp>

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

namespace io { namespace win { namespace detail {

constexpr void* void_cast(const uint8_t* ptr) {
	return const_cast<void*>( static_cast<const void*>(ptr) );
}

enum class whence_type: ::DWORD {
	current = FILE_CURRENT,
	begin = FILE_BEGIN,
	end = FILE_END
};

class handle_synch_channel
{
	handle_synch_channel(const handle_synch_channel&) = delete;
	handle_synch_channel& operator=(handle_synch_channel&) = delete;

	public:

		constexpr handle_synch_channel(::HANDLE hnd) noexcept:
			hnd_(hnd)
		{}

		~handle_synch_channel() noexcept
		{
			::CloseHandle(hnd_);
		}

		inline std::size_t read(std::error_code& err, uint8_t* const buff, std::size_t bytes) const noexcept
		{
			::DWORD result;
			if( ! ::ReadFile(hnd_, void_cast(buff), bytes, &result, nullptr) ) {
				err.assign(::GetLastError(), std::system_category() );
			}
			return result;
		}

		inline std::size_t write(std::error_code& err, const uint8_t* buff,std::size_t size) const noexcept
		{
			::DWORD result;
			if ( ! ::WriteFile(hnd_, void_cast(buff), size, &result, nullptr) ) {
				err.assign(::GetLastError(), std::system_category() );
			}
			return result;
		}

		inline uint64_t seek(std::error_code err,whence_type whence,int64_t offset)
		{
			::LARGE_INTEGER li;
			li.QuadPart = offset;
			if( !::SetFilePointerEx(hnd_, li, &li, static_cast<::DWORD>(whence) ) ) {
				err.assign(::GetLastError(), std::system_category() );
			}
			return li.QuadPart;
		}

		inline ::HANDLE hnd() noexcept {
			return hnd_;
		}

	private:
		::HANDLE hnd_;
};

}}} // namespace detail

#endif // _IO_WIN_HANDLECHANNEL_HPP_INCLUDED__
