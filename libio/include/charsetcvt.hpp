/*
 *
 * Copyright (c) 2016-2022
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef __IO_CHARSETS_CVT_HPP_INCLUDED__
#define __IO_CHARSETS_CVT_HPP_INCLUDED__

#include "config.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include "channels.hpp"
#include "charsets.hpp"
#include "scoped_array.hpp"
#include "type_traits_ext.hpp"

#include <system_error>

#ifndef iconv_t
#define iconv_t libiconv_t
typedef void* iconv_t;
#endif // iconv_t

namespace io {

namespace utf8 {

namespace detail {

static constexpr const char32_t SH2    = 6;
static constexpr const char32_t SH3    = 12;
static constexpr const char32_t SH4    = 18;

static constexpr const uint8_t TAIL_MASK  = 0x3F;
static constexpr const uint8_t B2_MASK    = 0x1F;
static constexpr const uint8_t B3_MASK    = 0x0F;
static constexpr const uint8_t B4_MASK    = 0x07;

#ifdef IO_HAS_CONNCEPTS
template<typename T >
	requires( is_charater_v<T> )
#else
template<
	typename T,
	typename std::enable_if<
		is_charater<T>::value
		>::type* = nullptr
	>
#endif // IO_HAS_CONNCEPTS
constexpr unsigned int make_uint(const T c) noexcept
{
	return static_cast< unsigned int >( std::char_traits<T>::to_int_type(c) );
}

constexpr unsigned int make_uint(const int c) noexcept
{
	return static_cast<unsigned int>( c );
}

#ifdef IO_IS_LITTLE_ENDIAN
static constexpr const unsigned int MBSHIFT = (sizeof(unsigned int) * CHAR_BIT ) - CHAR_BIT;

constexpr char32_t make_char32(uint32_t c) noexcept
{
	return static_cast<char32_t>(c);
}
#else
// invert byte ordering in case of big endian
constexpr char32_t make_char32(uint32_t c) noexcept
{
	return static_cast<char32_t>( io_bswap32(c) );
}
#endif // IO_IS_LITTLE_ENDIAN

constexpr uint8_t make_byte(const char c) noexcept
{
	return static_cast<uint8_t>( c );
}


constexpr uint32_t tail(const char tail) noexcept
{
	return uint32_t( make_byte(tail) & TAIL_MASK );
}

constexpr char32_t decode2(const char* mb2) noexcept
{
	return make_char32( ( uint32_t( make_byte(mb2[0]) & B2_MASK) << SH2) + tail(mb2[1]) );
}

inline constexpr char32_t decode3(const char* mb3) noexcept
{
	return make_char32( (uint32_t(make_byte(mb3[0]) & B3_MASK) << SH3)  +
			(tail(mb3[1]) << SH2) + tail(mb3[2]) );
}

constexpr char32_t decode4(const char* mb4) noexcept
{
	return make_char32( (uint32_t(make_byte(mb4[0]) & B4_MASK) << SH4) +
			( tail(mb4[1]) << SH3) +
			( tail(mb4[2]) << SH2) +
			tail( mb4[3]) );
}

constexpr unsigned int OBMAX = 0x80;
constexpr uint8_t MASK  = 6; // 0000 0110
constexpr uint8_t RS = CHAR_BIT - 3; //

} // namespace detail

#ifdef IO_HAS_CHAR8_T
typedef char8_t u8char_t;
#else
typedef unsigned char u8char_t;
#endif // IO_HAS_CHAR8_T

/// Checks a byte is UTF-8 single byte character
constexpr bool isonebyte(const unsigned int c) noexcept
{
	return c < detail::OBMAX;
}


/// Decode UTF-8 2 bytes multi-byte sequence to full char32_t UNICODE representation
using detail::decode2;
/// Decode UTF-8 3 bytes multi-byte sequence to full char32_t UNICODE representation
using detail::decode3;
/// Decode UTF-8 4 bytes multi-byte sequence to full char32_t UNICODE representation
using detail::decode4;


#if defined(__GNUG__) || defined(__clang__)
constexpr unsigned int mblen(const u8char_t* mb) noexcept
#else
inline unsigned int mblen(const u8char_t* mb) noexcept
#endif // __GNUG__
{
	return isonebyte(detail::make_uint(*mb))
	? 1
	:
	// bit scan forward on inverted value gives number of leading multibyte bits
	// works much faster then mask check series on CPU which supports clz or bt instruction
	// (most of CPU supporting it)
#ifdef IO_IS_LITTLE_ENDIAN
	detail::make_uint( io_clz( ~( detail::make_uint(*mb) << detail::MBSHIFT  ) ) );
#else
	detail::make_uint( io_clz( ~detail::make_uint(*mb) ) );
#endif // IO_IS_LITTLE_ENDIAN
}

#if	defined(__GNUG__) || defined(__clang__)
constexpr unsigned int mblen(const char* mb) noexcept
#else
inline unsigned int mblen(const char* mb)
#endif // __GNUG__
{
	return isonebyte( *mb )
	? 1
	:
	// bit scan forward on inverted value gives number of leading multibyte bits
	// works much faster then mask check series on CPU which supports clz or bt instruction
	// (most of CPU supporting it)
#ifdef IO_IS_LITTLE_ENDIAN
	detail::make_uint( io_clz( ~( detail::make_uint(*mb) << detail::MBSHIFT  ) ) );
#else
	detail::make_uint( io_clz( ~detail::make_uint(*mb) ) );
#endif // IO_IS_LITTLE_ENDIAN
}

#ifndef _MSC_VER
/// Converts a UTF-8 single/multibyte character to full UNICODE UTF-32 value,
/// endianes depends on current CPU
/// \param dst destination UTF-32 character, or U'\0' when end of line reached or invalid source character value
/// \param src pointer to the UTF-8 character value
/// \return string position after src UTF-8 or nullptr when end of line reached or decoding failed
const char* IO_PUBLIC_SYMBOL mbtochar32(char32_t& dst, const char* src) noexcept;

const u8char_t* IO_PUBLIC_SYMBOL mbtochar32(char32_t& dst, const u8char_t* src) noexcept;
#else

/// Converts a UTF-8 single/multibyte character to full UNICODE UTF-32 value,
/// endianes depends on current CPU
/// \param dst destination UTF-32 character, or U'\0' when end of line reached or invalid source character value
/// \param src pointer to the UTF-8 character value
/// \return string position after src UTF-8 or nullptr when end of line reached or decoding failed
IO_PUBLIC_SYMBOL const char* mbtochar32(char32_t& dst, const char* src) noexcept;

IO_PUBLIC_SYMBOL const u8char_t* mbtochar32(char32_t& dst, const u8char_t* src) noexcept;

#endif

/// Returns UTF-8 string length in logical UNICODE characters
/// \param u8str source UTF-8 string
/// \return length in characters
#if ( defined(__GNUG__) || defined(__clang__) ) && defined(__HAS_CPP_14)
constexpr
#else
inline
#endif
std::size_t strlength(const char* u8str) noexcept {
	std::size_t ret = 0;
	for(const char *c = u8str; '\0' != *c; c += mblen(c) )
		++ret;
	return ret;
}

} // namespace u8


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

// inside header to avoid exporting std::basic_string<char> i.e. std::string into shared library
// and avoid possible issue with different memory allocators like jemalloc etc
static std::string transcode_big(const wchar_t* ucs_str, std::size_t len)
{

	const sys_widechar_t* ucs = reinterpret_cast<const sys_widechar_t*>(ucs_str);
	scoped_arr<uint8_t> arr( detail::utf8_buff_size(ucs, len) );
	std::error_code ec;
	std::size_t conv = transcode(ec, ucs, len, arr.begin(), arr.len() );
	check_error_code(ec);
	return std::string( reinterpret_cast<char*>(arr.begin()), conv);
}

static std::string transcode_small(const wchar_t* ucs_str, std::size_t len)
{
#ifdef __IO_WINDOWS_BACKEND__
	const char16_t* ucs = reinterpret_cast<const char16_t*>(ucs_str);
	uint8_t tmp[512] = { 0 };
#else
	const char32_t* ucs = reinterpret_cast<const char32_t*>(ucs_str);
	uint8_t tmp[256] = { 0 };
#endif // __IO_WINDOWS_BACKEND__
	const std::size_t buff_size = utf8_buff_size(ucs, len);
	std::error_code ec;
	std::size_t conv = transcode(ec, ucs, len, tmp, buff_size);
	check_error_code(ec);
	return std::string(reinterpret_cast<char*>(tmp), conv);
}

} // namespace detail

class IO_PUBLIC_SYMBOL code_cnvtr;
DECLARE_IPTR(code_cnvtr);

/// \brief Character set conversation (transcoding) interface
class IO_PUBLIC_SYMBOL code_cnvtr final: public object {
public:
	/// Cursor character transcoding API, direct access to conversation engine
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
	static s_code_cnvtr open(std::error_code& ec,const charset& from,const charset& to, cnvrt_control conrol) noexcept;
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
	scoped_arr<uint8_t> arr( detail::utf8_buff_size(u16_str, len)  );
	std::error_code ec;
	std::size_t conv = transcode(ec, u16_str, len, arr.begin(), arr.len() );
	check_error_code(ec);
	return std::string( reinterpret_cast<char*>(arr.begin()), conv);
}

/// Convert a system UCS-4 character array to UTF-8 encoded STL string
inline std::string transcode(const char32_t* u32_str, std::size_t len)
{
	scoped_arr<uint8_t> arr( detail::utf8_buff_size(u32_str, len) );
	std::error_code ec;
	std::size_t conv = transcode(ec, u32_str, len, arr.begin(), arr.len() );
	check_error_code(ec);
	return std::string( reinterpret_cast<char*>(arr.begin()), conv);
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
	std::size_t conv = transcode(ec, reinterpret_cast<const uint8_t*>(u8_str), bytes, arr.begin(), arr.len() );
	check_error_code(ec);
	return std::wstring(reinterpret_cast<const wchar_t*>(arr.begin()), conv );
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

} // namespace io

namespace std {

template<>
struct is_error_condition_enum<io::converrc> : public true_type
{};

} // namespace std

#endif // __IO_CHARSETS_CVT_HPP_INCLUDED__
