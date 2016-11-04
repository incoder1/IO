/*
 * Copyright (c) 2016
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef __IO_TEXT_HPP_INCLUDED__
#define __IO_TEXT_HPP_INCLUDED__

#include "config.hpp"
#include "channels.hpp"
#include "charsetcvt.hpp"
#include "codedetect.hpp"

#include <atomic>
#include <exception>
#include <string>

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#ifdef __IO_WINDOWS_BACKEND__
#	define IO_U_TEXT(S) u##S;
#	define IO_UCHARSET io::code_pages::UTF_16LE
typedef char16_t unicode_char;
#define IO_UCONV_RATE  io::charset_conv_rate::twice_less
#endif // __IO_WINDOWS_BACKEND__

#ifdef __IO_POSIX_BACKEND__
#	define IO_U_TEXT(S) U##S
#	ifdef IO_IS_LITTLE_ENDIAN
#		define IO_UCHARSET io::code_pages::UTF_32LE
#	else
#		define IO_UCHARSET io::code_pages::UTF_32BE
#	endif // IO_IS_LITTLE_ENDIAN
typedef char32_t unicode_char;
#define IO_UCONV_RATE io::charset_conv_rate::same
#endif // __IO_WINDOWS_BACKEND__

namespace io {


/// Character set memory buffer size conversion rating
enum class charset_conv_rate: int8_t {
	/// use same transcoding memory buffer length
	/// e.g. CP_1250 -> UTF-8 || USC4 -> UTF-8
	same = 1,
	/// use two times less transcoding destination buffer length
	/// e.g. USC-2 -> UTF-8, UCS-4 -> UCS-2
	twice_less = -2,
	/// use four times larger transcoding destination buffer length
	/// e.g. UTF-8 -> UCS-4  || CP_1250 -> USC-4
	quad_larger = 4
};

/// \brief Onfly character conversation synchronous read channel implementation.
/*!
 * Reads bytes into internal memory buffer first, and then transcode it using embedded converter
 * WARNING! Do not use this channel for reading binary data
 !*/
class IO_PUBLIC_SYMBOL conv_read_channel final: public read_channel {
private:
	friend class io::nobadalloc<conv_read_channel>;
	conv_read_channel(s_read_channel&& src,code_cnvtr&& conv,int8_t crate) noexcept;
public:
	/// Opens a converting channel from a underlying read channel
	/// Note! Reentrant and thread safe, blocks on malloc and read operations.
	/// \param ec operation error code contains failure in case of:
	///         <ul>
	///           <li>character set conversion error</li>
	///           <li>io error</li>
	///           <li>out of memory</li>
	///         </ul>
	/// \param src source source raw bytes synchronous read channel
	/// \param conv character set converter
	/// \param crate transcoding buffers rating
	/// \throw never throws
	static s_read_channel open(std::error_code& ec,s_read_channel&& src,code_cnvtr&& conv,charset_conv_rate crate) noexcept;
	/// Reads bytes from underlying read channel and convert them to the destination charset
	/// \param ec operation error code
	/// \param buff destination memory buffer, must be at least bytes wide
	/// \param bytes requested bytes to read
	/// \throw never throws
	virtual std::size_t read(std::error_code& ec,uint8_t* const buff, std::size_t bytes) const noexcept override;
	/// Destroys channel and releases all associated resources
	/// \throw never throws
	virtual ~conv_read_channel() noexcept;
private:
	s_read_channel src_;
	code_cnvtr conv_;
	uint8_t crate_;
};
/// \brief On-fly character conversion synchronous write channel implementation.
/*!
	Convert characters into internal memory buffer first, and then writes them into destination channel.
	WARNING! Do not use this channel for binary output operations
!*/
class IO_PUBLIC_SYMBOL conv_write_channel final:public write_channel {
private:
	friend class io::nobadalloc<conv_write_channel>;
	conv_write_channel(s_write_channel&& dst,code_cnvtr&& conv,int8_t crate) noexcept;
public:
	/// Opens a converting channel from a underlying write channel
	/// Note! Reentrant and thread safe, blocks on malloc and read operations.
	/// \param ec operation error code contains failure in case of:
	///         <ul>
	///           <li>character set conversion error</li>
	///           <li>io error</li>
	///           <li>out of memory</li>
	///         </ul>
	/// \param dst destination synchronous write channel
	/// \param conv character set converter
	/// \param crate transcoding buffers rating
	/// \throw never throws
	static s_write_channel open(std::error_code& ec,s_write_channel&& dst,code_cnvtr&& conv,charset_conv_rate crate) noexcept;
	/// Destroys channel and releases all associated resources
	/// \throw never throws
	virtual ~conv_write_channel() noexcept override;
	/// Writes bytes to underlying write channel with converting them to the destination charset
	/// \param ec operation error code
	/// \param buff source characters array, must be at least bytes wide
	/// \param bytes requested bytes to write
	/// \throw never throws
	virtual std::size_t write(std::error_code& ec, const uint8_t* buff,std::size_t bytes) const noexcept override;
private:
	s_write_channel dst_;
	code_cnvtr conv_;
	uint8_t crate_;
};

namespace detail {
#ifndef IO_NO_EXCEPTIONS
	static inline void check_error(const std::error_code& ec) {
		if(ec)
			throw std::system_error(ec);
	}
#else
	static inline void check_error(const std::error_code& ec) {
		if(ec)
			std::unexpected();
	}
#endif // IO_NO_EXCEPTIONS
	template<typename T>
	struct scoped_arr
	{
		scoped_arr(const scoped_arr&)  = delete;
		scoped_arr& operator=(const scoped_arr&) = delete;
		constexpr scoped_arr(std::size_t len):
			len_(len),
			mem_ ( new T[len_] )
		{}
		~scoped_arr() noexcept {
			delete mem_;
		}
		inline T* get() const noexcept
		{
			return mem_;
		}
		inline std::size_t len() const noexcept {
			return len_;
		}
	private:
		const std::size_t len_;
		T* mem_;
	};

	template<typename T>
	static constexpr uint8_t* address_of(const T* p) noexcept {
		return const_cast<uint8_t*>( reinterpret_cast<const uint8_t*>(p) );
	}
} // namesapace detail


/// Convert an UTF-8 character array to system UCS-2 encoded char16_t string
/// \param u8_str a UTF-8 encoded character array, must be at least bytes wide
/// \param bytes requested bytes to convert
/// \retrun  system UCS-2 encoded char16_t STL string
/// \throw std::system_error or std::bad_alloc
inline std::u16string transcode_to_u16(const char* u8_str, std::size_t bytes)
{
	detail::scoped_arr<char16_t> arr(bytes);
	std::error_code ec;
	std::size_t conv = transcode(ec, detail::address_of(u8_str), bytes, arr.get(), arr.len() );
	detail::check_error(ec);
	return std::u16string(arr.get(), conv);
}

/// Convert an UTF-8 character array to system UCS-4 encoded char16_t string
/// \param u8_str a UTF-8 encoded character array, must be at least bytes wide
/// \param bytes requested bytes to convert
/// \retrun  system UCS-4 encoded char32_t STL string
/// \throw std::system_error or std::bad_alloc
inline std::u32string transcode_to_u32(const char* u8_str, std::size_t bytes)
{
	detail::scoped_arr<char32_t> arr(bytes);
	std::error_code ec;
	std::size_t conv = transcode(ec, detail::address_of(u8_str), bytes, arr.get(), arr.len() );
	detail::check_error(ec);
	return std::u32string(arr.get(), conv);
}

/// Convert an UTF-8 zero terminated string to system USC-2 encoded char16_t string
inline std::u16string transcode_to_u16(const char* u8_str)
{
	return transcode_to_u16( u8_str, std::char_traits<char>::length(u8_str) );
}

/// Convert an UTF-8 zero terminated string to system USC-2 encoded char32_t string
inline std::u32string transcode_to_u32(const char* u8_str)
{
	return transcode_to_u32( u8_str, std::char_traits<char>::length(u8_str) );
}

/// Convert a system UCS-2 character array to UTF-8 encoded STL string
inline std::string transcode(const char16_t* u16_str, std::size_t len) {
	detail::scoped_arr<char> arr( len*sizeof(char16_t) );
	std::error_code ec;
	std::size_t conv = transcode(ec, u16_str, len, detail::address_of(arr.get()), arr.len() );
	detail::check_error(ec);
	return std::string(arr.get(), conv);
}

/// Convert a system UCS-4 character array to UTF-8 encoded STL string
inline std::string transcode(const char32_t* u32_str, std::size_t len) {
	detail::scoped_arr<char> arr( len*sizeof(char32_t) );
	std::error_code ec;
	std::size_t conv = transcode(ec, u32_str, len,detail::address_of(arr.get()), arr.len() );
	detail::check_error(ec);
	return std::string(arr.get(), conv);
}

inline std::string transcode(const char16_t* u16_str)
{
	return transcode(u16_str, std::char_traits<char16_t>::length(u16_str) );
}

inline std::string transcode(const char32_t* u32_str)
{
	return transcode(u32_str, std::char_traits<char32_t>::length(u32_str) );
}

/// Convert UTF-8 encoded character array to
/// system UCS (UTF-(16|32)(LE|BE) ) character string
inline std::wstring transcode_to_ucs(const char* u8_str, std::size_t bytes) {
#ifdef __IO_WINDOWS_BACKEND__
	detail::scoped_arr<char16_t> arr( bytes*sizeof(char16_t) );
#else
	detail::scoped_arr<char32_t> arr( bytes*sizeof(char32_t) );
#endif // __IO_WINDOWS_BACKEND__
	std::error_code ec;
	std::size_t conv = transcode(ec, detail::address_of(u8_str), bytes, arr.get(), arr.len() );
	detail::check_error(ec);
	return std::wstring(reinterpret_cast<const wchar_t*>(arr.get()), conv );
}

inline std::wstring transcode_to_ucs(const char* u8_str) {
	return transcode_to_ucs(u8_str, std::char_traits<char>::length(u8_str) );
}

inline std::string transcode(const wchar_t* ucs_str, std::size_t len) {
#ifdef __IO_WINDOWS_BACKEND__
	const char16_t* ucs = reinterpret_cast<const char16_t*>(ucs_str);
	detail::scoped_arr<char> arr( len*2 );
#else
	const char32_t* ucs = reinterpret_cast<const char32_t*>(ucs_str);
	detail::scoped_arr<char> arr( len*4 );
#endif // __IO_WINDOWS_BACKEND__
	std::error_code ec;
	std::size_t conv = transcode(ec, ucs, len, detail::address_of(arr.get()), arr.len() );
	detail::check_error(ec);
	return std::string(arr.get(), conv);
}

inline std::string transcode(const wchar_t* ucs_str) {
	return transcode(ucs_str, std::char_traits<wchar_t>::length(ucs_str) );
}

} // namespace

#endif // __IO_TEXT_HPP_INCLUDED__
