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
#ifndef __IO_TEXT_HPP_INCLUDED__
#define __IO_TEXT_HPP_INCLUDED__

#include "config.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include "channels.hpp"
#include "charsetcvt.hpp"
#include "unicode_bom.hpp"
#include "errorcheck.hpp"

#include <atomic>
#include <exception>
#include <string>

#ifdef __IO_WINDOWS_BACKEND__
#	define IO_SYS_UNICODE io::code_pages::UTF_16LE
typedef wchar_t unicode_char;
#endif // __IO_WINDOWS_BACKEND__

#ifdef __IO_POSIX_BACKEND__
#	ifdef IO_IS_LITTLE_ENDIAN
#		define IO_SYS_UNICODE io::code_pages::UTF_32LE
#	else
#		define IO_SYS_UNICODE io::code_pages::UTF_32BE
#	endif // IO_IS_LITTLE_ENDIAN
typedef char32_t unicode_char;
#endif // __IO_WINDOWS_BACKEND__

namespace io {

/// \brief On-fly character conversation synchronous read channel implementation.
/*!
 * Reads bytes into internal memory buffer first, and then transcode it using embedded converter
 * WARNING! Do not use this channel for reading binary data
 !*/
class IO_PUBLIC_SYMBOL conv_read_channel final: public read_channel {
private:
	friend class io::nobadalloc<conv_read_channel>;
	conv_read_channel(const s_read_channel& src,s_code_cnvtr&& conv) noexcept;
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
	/// \param control character conversation behavior control
	/// \throw never throws
	static s_read_channel open(std::error_code& ec,const s_read_channel& src,
							   const charset& from,
							   const charset& to,
							   const cnvrt_control control) noexcept;

	static s_read_channel open(std::error_code& ec, const s_read_channel& src,const s_code_cnvtr& conv) noexcept;

	/// Reads bytes from underlying read channel and convert them to the destination charset
	/// \param ec operation error code
	/// \param buff destination memory buffer, must be at least bytes wide
	/// \param bytes requested bytes to read
	/// \throw never throws
	virtual std::size_t read(std::error_code& ec,uint8_t* const buff, std::size_t bytes) const noexcept override;
	/// Destroys channel and releases all associated resources
	/// \throw never throws
	virtual ~conv_read_channel() noexcept = default;
private:
	s_read_channel src_;
	s_code_cnvtr conv_;
};

/// \brief On-fly character conversion synchronous write channel implementation.
/*!
	Convert characters into internal memory buffer first, and then writes them into destination channel.
	WARNING! Do not use this channel for binary output operations
!*/
class IO_PUBLIC_SYMBOL conv_write_channel final:public write_channel {
private:
	friend class io::nobadalloc<conv_write_channel>;
	conv_write_channel(const s_write_channel& dst,const s_code_cnvtr& conv) noexcept;
	std::size_t convert_some(std::error_code& ec, const uint8_t *src, std::size_t &size, uint8_t *dst) const;
public:
	static s_write_channel open(std::error_code& ec,
								const s_write_channel& dst,
								const s_code_cnvtr& conv) noexcept;
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
	s_code_cnvtr conv_;
};

namespace detail {

template<typename T>
static constexpr uint8_t* byte_cast(const T* p) noexcept
{
	return const_cast<uint8_t*>( reinterpret_cast<const uint8_t*>(p) );
}

// returns UTF-8 character size in bytes
static unsigned int u8_mblen(const char* mb) noexcept
{
	if( static_cast<unsigned int>(mb[0]) < 0x80U)
		return 1;
#ifdef IO_IS_LITTLE_ENDIAN
	static constexpr unsigned int MB_SHIFT = ( sizeof(unsigned int) << 3 ) - 8;
	unsigned int c = static_cast<unsigned int>(mb[0]) << MB_SHIFT;
#else
	unsigned int c = static_cast<unsigned int>(mb[0]);
#endif // IO_IS_LITTLE_ENDIAN
	return static_cast<unsigned int>( io_clz( ~c ) );
}

static std::size_t utf16_buff_size(const char* b, std::size_t size) noexcept
{
	const char *end = b + size;
	std::size_t ret = 0;
	const char *c = b;
	unsigned int mblen;
	while( (b < end) && '\0' != *c) {
		mblen = u8_mblen(c);
		ret = ret > 2 ? ret + 2 : ret + 1;
		c = c + mblen;
	}
	return ret;
}

static std::size_t utf32_buff_size(const char* b, std::size_t size) noexcept
{
	const char *end = b + size;
	std::size_t ret = 0;
	const char *c = b;
	unsigned int mblen;
	while( (b < end) && '\0' != *c) {
		mblen = u8_mblen(c);
		ret += mblen;
		c = c + mblen;
	}
	return ret;
}

static constexpr uint16_t LATIN1_MAX = 0x80;
static constexpr uint16_t TWOB_MAX = 0x800;
static constexpr uint16_t SURROGATE_LEAD_FIRST = 0xDC00;
static constexpr uint16_t SURROGATE_TRAIL_LAST = 0xDF;
static constexpr uint32_t THREEB_MAX = 0x10000;

static constexpr bool is_surogate_word(uint16_t ch) noexcept {
	return ch >= SURROGATE_LEAD_FIRST && ch <= SURROGATE_TRAIL_LAST;
}

static std::size_t utf8_buff_size(const char16_t* ustr, std::size_t size) noexcept
{
	std::size_t ret = 0;
	const uint16_t* c = reinterpret_cast<const uint16_t*>( ustr );
	for (std::size_t i = 0; *c && (i < size); i++, c++) {
		if (*c < LATIN1_MAX)
			++ret;
		else if (*c < TWOB_MAX) {
			ret += 2;
		}
		else if ( is_surogate_word(*c) ) {
			ret += 4;
			++i;
		}
		else
			ret += 3;
	}
	return ret;
}

static std::size_t utf8_buff_size(const char32_t* ustr, std::size_t size) noexcept
{
	std::size_t ret = 0;
    const uint32_t* c = reinterpret_cast<const uint32_t*>(ustr);
	for(std::size_t i = 0; *c && (i < size); i++, c++) {
		if(*c < LATIN1_MAX)
			++ret;
		else if(*c < TWOB_MAX)
			ret += 2;
		else if(*c < THREEB_MAX)
			ret += 3;
		else
			ret += 4;
	}
	return ret;
}

static std::string transcode_big(const wchar_t* ucs_str, std::size_t len)
{
#ifdef __IO_WINDOWS_BACKEND__
	const char16_t* ucs = reinterpret_cast<const char16_t*>(ucs_str);
#else
	const char32_t* ucs = reinterpret_cast<const char32_t*>(ucs_str);
#endif // __IO_WINDOWS_BACKEND__
	scoped_arr<char> arr( detail::utf8_buff_size(ucs, len) );
	std::error_code ec;
	std::size_t conv = transcode(ec, ucs, len, detail::byte_cast(arr.get()), arr.len() );
	check_error_code(ec);
	return std::string(arr.get(), conv);
}

static std::string transcode_small(const wchar_t* ucs_str, std::size_t len)
{
#ifdef __IO_WINDOWS_BACKEND__
	const char16_t* ucs = reinterpret_cast<const char16_t*>(ucs_str);
#else
	const char32_t* ucs = reinterpret_cast<const char32_t*>(ucs_str);
#endif // __IO_WINDOWS_BACKEND__
	const std::size_t buff_size = detail::utf8_buff_size(ucs, len);
	uint8_t *tmp = static_cast<uint8_t*>( io_alloca( buff_size ) );
	std::error_code ec;
	std::size_t conv = transcode(ec, ucs, len, tmp, buff_size );
	check_error_code(ec);
	return std::string( reinterpret_cast<const char*>(tmp), conv);
}

} // namesapace detail


/// Convert an UTF-8 character array to system UCS-2 encoded char16_t string
/// \param u8_str a UTF-8 encoded character array, must be at least bytes wide
/// \param bytes requested bytes to convert
/// \retrun  system UCS-2 encoded char16_t STL string
/// \throw std::system_error or std::bad_alloc
inline std::u16string transcode_to_u16(const char* u8_str, std::size_t bytes)
{
	scoped_arr<char16_t> arr( detail::utf16_buff_size(u8_str, bytes) + 1 );
	std::error_code ec;
	std::size_t conv = transcode(ec, detail::byte_cast(u8_str), bytes, arr.get(), arr.len() );
	check_error_code(ec);
	return std::u16string(arr.get(), conv);
}

/// Convert an UTF-8 character array to system UCS-4 encoded char16_t string
/// \param u8_str a UTF-8 encoded character array, must be at least bytes wide
/// \param bytes requested bytes to convert
/// \retrun  system UCS-4 encoded char32_t STL string
/// \throw std::system_error or std::bad_alloc
inline std::u32string transcode_to_u32(const char* u8_str, std::size_t bytes)
{
	scoped_arr<char32_t> arr( detail::utf32_buff_size(u8_str,bytes) + 1);
	std::error_code ec;
	std::size_t conv = transcode(ec, detail::byte_cast(u8_str), bytes, arr.get(), arr.len() );
	check_error_code(ec);
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
inline std::string transcode(const char16_t* u16_str, std::size_t len)
{
	scoped_arr<char> arr( detail::utf8_buff_size(u16_str, len)  );
	std::error_code ec;
	std::size_t conv = transcode(ec, u16_str, len, detail::byte_cast(arr.get()), arr.len() );
	check_error_code(ec);
	return std::string(arr.get(), conv);
}

/// Convert a system UCS-4 character array to UTF-8 encoded STL string
inline std::string transcode(const char32_t* u32_str, std::size_t len)
{
	scoped_arr<char> arr( detail::utf8_buff_size(u32_str, len) );
	std::error_code ec;
	std::size_t conv = transcode(ec, u32_str, len,detail::byte_cast(arr.get()), arr.len() );
	check_error_code(ec);
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
inline std::wstring transcode_to_ucs(const char* u8_str, std::size_t bytes)
{
#ifdef __IO_WINDOWS_BACKEND__
	scoped_arr<char16_t> arr( detail::utf16_buff_size(u8_str, bytes) + 1 );
#else
	scoped_arr<char32_t> arr( detail::utf32_buff_size(u8_str,bytes) + 1);
#endif // __IO_WINDOWS_BACKEND__
	std::error_code ec;
	std::size_t conv = transcode(ec, detail::byte_cast(u8_str), bytes, arr.get(), arr.len() );
	check_error_code(ec);
	return std::wstring(reinterpret_cast<const wchar_t*>(arr.get()), conv );
}

inline std::wstring transcode_to_ucs(const char* u8_str)
{
	return transcode_to_ucs(u8_str, std::char_traits<char>::length(u8_str) );
}

inline std::string transcode(const wchar_t* ucs_str, std::size_t len)
{
	return ( io_likely( len <= (1024 / sizeof(wchar_t) ) ) )
		? detail::transcode_small(ucs_str, len)
		: detail::transcode_big(ucs_str, len);
}

inline std::string transcode(const wchar_t* ucs_str)
{
	return transcode(ucs_str, std::char_traits<wchar_t>::length(ucs_str) );
}


template<typename> struct __is_char_type_helper : public std::false_type {};
template<> struct __is_char_type_helper<char> : public std::true_type {};
template<> struct __is_char_type_helper<wchar_t> : public std::true_type {};
template<> struct __is_char_type_helper<char16_t> : public std::true_type {};
template<> struct __is_char_type_helper<char32_t> : public std::true_type {};
#ifdef IO_HAS_CHAR8_T
template<> struct __is_char_type_helper<char8_t> : public std::true_type {};
#endif

template<typename _Tp>
struct is_char_type : public __is_char_type_helper<typename std::remove_cv<_Tp>::type>::type
{};

template<typename C, class ___type_restriction = void>
class basic_reader;

template<typename C>
class basic_reader<C, typename std::enable_if< is_char_type< C >::value >::type >
{
private:
	static constexpr std::size_t CSIZE = sizeof(C);
public:
	explicit basic_reader(const s_read_channel& src) noexcept:
		src_( src )
	{}
	inline std::size_t read(std::error_code& ec, C* to, std::size_t chars) const noexcept {
		return CSIZE * src_->read(ec, reinterpret_cast<uint8_t*>(to), (chars * CSIZE ) );
	}
private:
	s_read_channel src_;
};

template<typename C, class ___type_restriction = void>
class basic_writer;

template<typename C>
class basic_writer<C, typename std::enable_if< is_char_type< C >::value >::type >
{
	basic_writer(const basic_writer&&) = delete;
	basic_writer operator=(const basic_writer&&) = delete;
private:
	static constexpr std::size_t CSIZE = sizeof(C);
private:
	void flush()
	{
		buffer_.flip();
		std::size_t written = 0;
		std::size_t left = buffer_.length();
		while( !ec_ && 0 != left ) {
			written = dst_->write(ec_, buffer_.position().get(), left );
			buffer_.shift(written);
			left = buffer_.length();
		}
		buffer_.clear();
	}
public:
	typedef std::char_traits<C> char_traits;
	basic_writer(s_write_channel&& dst,io::byte_buffer &&buffer) noexcept:
		dst_(  std::forward<s_write_channel>(dst) ),
		buffer_( std::forward<byte_buffer>(buffer) )
	{}

	basic_writer(basic_writer&& other) noexcept:
		dst_(),
		buffer_(),
		ec_()
	{
		dst_ = std::move(other.dst_);
		buffer_ = std::move(other.buffer_);
		ec_ = std::move(other.ec_);
	}

	basic_writer& operator=(basic_writer&& other) noexcept
	{
		basic_writer( other ).swap( *this );
		return *this;
	}

	~basic_writer() noexcept
	{
		if(dst_ && !buffer_.empty() && !ec_ ) {
			buffer_.flip();
			std::size_t written = 0;
			std::size_t left = buffer_.length();
			while( !ec_ && 0 != left ) {
				written = dst_->write(ec_, buffer_.position().get(), left );
				buffer_.shift(written);
				left = buffer_.length();
			}
		}
	}

	void write(C ch) noexcept
	{
		if( buffer_.full() )
			flush();
		buffer_.put(ch);
	}

	void write(const C* str, std::size_t len) noexcept
	{
		if( buffer_.available() < (len*CSIZE) )
			flush();
		buffer_.put(str, len);
	}

	void write(const C* str) noexcept
	{
		return write(str, char_traits::length(str) );
	}

	void write(const std::basic_string<C>& str ) noexcept
	{
		return write( str.data(), str.size() );
	}

	explicit operator bool() const noexcept
	{
		return 0 == ec_.value();
	}

	std::error_code last_error() const noexcept
	{
		return ec_;
	}

	void swap(basic_writer& other) noexcept
	{
		dst_.swap( other.dst_ );
		buffer_.swap(other.buffer_);
		std::swap(ec_, other.ec_);
	}

private:
	s_write_channel dst_;
	byte_buffer buffer_;
	std::error_code ec_;
};

typedef basic_reader<char> reader;
typedef basic_reader<wchar_t> wreader;
typedef basic_reader<char16_t> u16_reader;
typedef basic_reader<char32_t> u32_reader;
typedef basic_writer<char> writer;
typedef basic_writer<wchar_t> wwriter;
typedef basic_writer<char16_t> u16_writer;
typedef basic_writer<char32_t> u32_writer;
#ifdef IO_HAS_CHAR8_T
typedef basic_writer<char8_t> u8_writer;
typedef basic_reader<char8_t> u8_reader;
#endif // IO_HAS_CHAR8_T

} // namespace io

#endif // __IO_TEXT_HPP_INCLUDED__
