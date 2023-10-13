/*
 *
 * Copyright (c) 2016-2023
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef __IO_CHARSETS_CVT_HPP_INCLUDED__
#define __IO_CHARSETS_CVT_HPP_INCLUDED__

#include <io/config/libio_config.hpp>

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <io/core/channels.hpp>
#include <io/core/type_traits_ext.hpp>

#include "unicode.hpp"
#include "charsets.hpp"

#ifndef iconv_t
#define iconv_t libiconv_t
typedef void* iconv_t;
#endif // iconv_t

namespace io {

/// Convert a character array UTF-8 encoded to platform current USC-2 (UTF-16LE or UTF-16BE) character array
/// \param  ec operation error code
/// \param  u8_scr source character array UTF-8 encoded, must not be nullptr and at least src_bytes wide
/// \param  src_bytes size of source array in bytes
/// \param  dst destination memory buffer, must not be nullptr and at least dst_size elements wide
/// \param  dst_size size of destination memory buffer available elements in char16_t elements
/// \return count char16_t elements in destination array, or 0 in case of failure
/// \throw never throws
std::size_t IO_PUBLIC_SYMBOL transcode(std::error_code& ec,const uint8_t* u8_src, std::size_t src_bytes, char16_t* const u16_dst, std::size_t dst_size) noexcept;

/// Convert a character array UTF-8 encoded to platform current UCS-4 (UTF-32LE or UTF-32BE) character array
/// \param  ec operation error code
/// \param  u8_src source character array UTF-8 encoded
/// \param  src_bytes size of source array in bytes
/// \param  u32_dst destination memory buffer, must not be nullptr and at least dst_size width
/// \param  dst_size size of destination memory buffer available char32_t elements
/// \return count of filled char32_t elements in destination array, or 0 in case of failure
/// \throw never throws
std::size_t IO_PUBLIC_SYMBOL transcode(std::error_code& ec,const uint8_t* u8_src, std::size_t src_bytes, char32_t* const u32_dst, std::size_t dst_size) noexcept;

/// Converts platform current UCS-2 (UTF-16LE or UTF-16BE) character array to UTF-8 encoded character array
/// \param  ec operation error code
/// \param  u16_src source character array UCS-2 encoded must not be nullptr and at least src_width wide
/// \param  src_width size of source array in char16_t elements
/// \param  dst destination memory buffer, must not be nullptr and at least dst_size bytes wide
/// \param  dst_size size of destination memory buffer available char16_t elements
/// \return count of filled bytes in destination character array, or 0 in case of failure
/// \throw never throws
std::size_t IO_PUBLIC_SYMBOL transcode(std::error_code& ec,const char16_t* u16_src, std::size_t src_width, uint8_t* const u8_dst, std::size_t dst_size) noexcept;


/// Converts platform current UCS-4 (UTF-32LE or UTF-32BE) character array to UTF-8 encoded character array
/// \param  ec operation error code
/// \param  u32_src source character array UCS-4 encoded
/// \param  src_width size of source array in char32_t elements
/// \param  dst destination memory buffer, must not be nullptr and at least dst_size wide
/// \param  dst_size size of destination memory buffer available elements in char32_t elements
/// \return count of filled bytes in destination character array, or 0 in case of failure
/// \throw never throws
std::size_t IO_PUBLIC_SYMBOL transcode(std::error_code& ec,const char32_t* u32_src, std::size_t src_width, uint8_t* const u8_dst, std::size_t dst_size) noexcept;


/// \brief Character set conversation (transcoding) error code
enum converrc: int {
	/// conversation was success
	success = 0,

	/// character code conversion invalid multi-byte sequence
	invalid_multibyte_sequence = 1,

	/// character code conversion incomplete multi-byte sequence
	incomplete_multibyte_sequence = 2,

	/// character code conversion no more room i.e. output buffer overflow
	no_buffer_space = 3,

	/// conversation between character's is not supported
	not_supported = 4,

	/// An unknown error
	unknown = -1
};

class IO_PUBLIC_SYMBOL chconv_error_category final: public std::error_category {
private:
#ifdef IO_DELCSPEC
	friend IO_PUBLIC_SYMBOL std::error_code  make_error_code(io::converrc ec) noexcept;
	friend IO_PUBLIC_SYMBOL std::error_condition  make_error_condition(io::converrc err) noexcept;
#else
	friend IO_PUBLIC_SYMBOL std::error_code make_error_code(io::converrc errc) noexcept;
	friend IO_PUBLIC_SYMBOL std::error_condition make_error_condition(io::converrc err) noexcept;
#endif
	static const chconv_error_category* instance();

	const char* cstr_message(int err_code) const;

public:

	chconv_error_category() noexcept:
		std::error_category()
	{}

	virtual ~chconv_error_category() = default;

	virtual const char* name() const noexcept override;

	virtual std::error_condition default_error_condition (int err) const noexcept override;

	virtual bool equivalent (const std::error_code& code, int condition) const noexcept override;

	virtual std::string message(int err_code) const override
	{
		return std::string(  cstr_message(err_code) );
	}

};

#ifdef IO_DELCSPEC
IO_PUBLIC_SYMBOL std::error_code make_error_code(io::converrc errc) noexcept;
IO_PUBLIC_SYMBOL std::error_condition make_error_condition(io::converrc err) noexcept;
#else
std::error_code IO_PUBLIC_SYMBOL make_error_code(io::converrc errc) noexcept;
std::error_condition  IO_PUBLIC_SYMBOL make_error_condition(io::converrc err) noexcept;
#endif // IO_DELCSPEC

enum class cnvrt_control
{
	failure_on_failing_chars,
	discard_on_failing_chars
};

// private transcoding functionality
namespace detail {

// iconv engine wrapper
class IO_PUBLIC_SYMBOL engine {
public:
	engine(const engine&) = delete;
	engine& operator=(const engine&) = delete;

	engine(engine&& other) noexcept;
	engine& operator=(engine&& rhs) noexcept;

	explicit operator bool() const {
		return is_open();
	}

	engine() noexcept;
	engine(const char* from,const char* to, cnvrt_control control) noexcept;
	~engine() noexcept;

	inline void swap(engine& other) noexcept;
	bool is_open() const;

	converrc convert(const uint8_t** src,std::size_t& size, uint8_t** dst, std::size_t& avail) const noexcept;
private:
	iconv_t iconv_;
};

#ifdef IO_DELCSPEC
IO_PUBLIC_SYMBOL std::size_t utf8_buff_size(const char16_t* ustr, std::size_t size) noexcept;
IO_PUBLIC_SYMBOL std::size_t utf8_buff_size(const char32_t* ustr, std::size_t size) noexcept;
IO_PUBLIC_SYMBOL std::size_t utf16_buff_size(const char* b, std::size_t size) noexcept;
IO_PUBLIC_SYMBOL std::size_t utf32_buff_size(const char* b, std::size_t size) noexcept;
#else
std::size_t IO_PUBLIC_SYMBOL utf8_buff_size(const char16_t* ustr, std::size_t size) noexcept;
std::size_t IO_PUBLIC_SYMBOL utf8_buff_size(const char32_t* ustr, std::size_t size) noexcept;
std::size_t IO_PUBLIC_SYMBOL utf16_buff_size(const char* b, std::size_t size) noexcept;
std::size_t IO_PUBLIC_SYMBOL utf32_buff_size(const char* b, std::size_t size) noexcept;
#endif // IO_DELCSPEC

#ifdef __IO_WINDOWS_BACKEND__
typedef char16_t sys_widechar_t;
#else
typedef char32_t sys_widechar_t;
#endif // __IO_WINDOWS_BACKEND__

} // namespace detail

class IO_PUBLIC_SYMBOL code_cnvtr;
DECLARE_IPTR(code_cnvtr);

/// \brief Character set conversation (transcoding) interface
class IO_PUBLIC_SYMBOL code_cnvtr final: public object {
public:
	/// Cursor character transcoding API, direct access to character set conversion engine
	/// \param ec
	///    operation error code
	/// \param in
	///    source block of input character sequence, as raw bytes
	/// \param in_bytes_left
	///
	/// \param out
	void convert(std::error_code& ec, const uint8_t** in,std::size_t& in_bytes_left,uint8_t** const out, std::size_t& out_bytes_left) const noexcept;

	void convert(std::error_code& ec, const uint8_t* src,const std::size_t src_size, byte_buffer& dst) const noexcept;

private:
	friend class nobadalloc<code_cnvtr>;
	code_cnvtr(detail::engine&& eng) noexcept;
public:
	static s_code_cnvtr open(std::error_code& ec,const charset* from,const charset* to, cnvrt_control conrol) noexcept;
	static std::error_condition _ok;
private:
	detail::engine eng_; // converting engine
};

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
	///		 <ul>
	///			<li>character set conversion error</li>
	///			<li>io error</li>
	///			<li>out of memory</li>
	///		</ul>
	/// \param src source source raw bytes synchronous read channel
	/// \param conv character set converter
	/// \param control character conversation behavior control
	/// \throw never throws
	static s_read_channel open(std::error_code& ec,const s_read_channel& src,
								const charset* from,
								const charset* to,
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

/// Convert an UTF-8 character array to system UCS-2 encoded char16_t string
/// \param u8_str a UTF-8 encoded character array, must be at least bytes wide
/// \param bytes requested bytes to convert
/// \retrun  system UCS-2 encoded char16_t STL string
/// \throw std::system_error or std::bad_alloc
inline std::u16string transcode_to_u16(const char* u8_str, std::size_t bytes)
{
	scoped_arr<char16_t> arr( detail::utf16_buff_size(u8_str, bytes) + 1 );
	std::error_code ec;
	std::size_t conv = transcode(ec, reinterpret_cast<const uint8_t*>(u8_str), bytes, arr.begin(), arr.len() );
	check_error_code(ec);
	return std::u16string(arr.begin(), conv);
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
	std::size_t conv = transcode(ec, reinterpret_cast<const uint8_t*>(u8_str), bytes, arr.begin(), arr.len() );
	check_error_code(ec);
	return std::u32string(arr.begin(), conv);
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
	std::size_t buff_size = detail::utf8_buff_size(u16_str, len);
	std::error_code ec;
	std::string ret(buff_size, '\0');
	std::size_t conv = transcode(ec, u16_str, len, reinterpret_cast<uint8_t*>(&ret[0]), buff_size );
	check_error_code(ec);
	ret.resize(conv);
#ifdef __HAS_CPP_17
	ret.shrink_to_fit();
#endif // __HAS_CPP_17
	return ret;
}

/// Convert a system UCS-4 character array to UTF-8 encoded STL string
inline std::string transcode(const char32_t* u32_str, std::size_t len)
{
	std::size_t buff_size = detail::utf8_buff_size(u32_str, len);
	std::string ret(buff_size, '\0');
	std::error_code ec;
	std::size_t conv = transcode(ec, u32_str, len, reinterpret_cast<uint8_t*>(&ret[0]), buff_size );
	check_error_code(ec);
	ret.resize(conv);
#ifdef __HAS_CPP_17
	ret.shrink_to_fit();
#endif // __HAS_CPP_17
	return ret;
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
	std::size_t buff_size = detail::utf32_buff_size(u8_str,bytes) + 1;
	std::wstring ret(buff_size, '\0');
	std::error_code ec;
	std::size_t conv = transcode(ec, reinterpret_cast<const uint8_t*>(u8_str), bytes,  reinterpret_cast<detail::sys_widechar_t*>(&ret[0]), buff_size );
	check_error_code(ec);
	ret.resize(conv);
#ifdef __HAS_CPP_17
	ret.shrink_to_fit();
#endif // __HAS_CPP_17
	return ret;
}

inline std::wstring transcode_to_ucs(const char* u8_str)
{
	return transcode_to_ucs(u8_str, std::char_traits<char>::length(u8_str) );
}

inline std::string transcode(const wchar_t* ucs_str, std::size_t len)
{
	const detail::sys_widechar_t* ucs = reinterpret_cast<const detail::sys_widechar_t*>(ucs_str);
	std::size_t buff_size = detail::utf8_buff_size(ucs, len);
	std::string ret(buff_size, '\0');
	std::error_code ec;
	std::size_t conv = transcode(ec, ucs, len, reinterpret_cast<uint8_t*>(&ret[0]), buff_size );
	check_error_code(ec);
	ret.resize(conv);
#ifdef __HAS_CPP_17
	ret.shrink_to_fit();
#endif // __HAS_CPP_17
	return ret;
}

inline std::string transcode(const wchar_t* ucs_str)
{
	return transcode(ucs_str, std::char_traits<wchar_t>::length(ucs_str) );
}

} // namespace io

namespace std {

template<>
struct is_error_condition_enum<io::converrc> : public true_type
{};

} // namespace std

#endif // __IO_CHARSETS_CVT_HPP_INCLUDED__
