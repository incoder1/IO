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
#ifndef __IO_CHARSETS_CVT_HPP_INCLUDED__
#define __IO_CHARSETS_CVT_HPP_INCLUDED__

#include <cerrno>
#include <functional>
#include <system_error>
#include <locale>

#include "buffer.hpp"
#include "charsets.hpp"
#include "object.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

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

#ifdef IO_IS_LITTLE_ENDIAN
static constexpr const unsigned int MBSHIFT = (sizeof(unsigned int) << 3 ) - 8;
#endif // IO_IS_LITTLE_ENDIAN

inline constexpr char tail(const uint8_t tail) noexcept
{
	return tail & TAIL_MASK;
}

inline constexpr const uint8_t* make_unsigned(const char* str) noexcept
{
	return reinterpret_cast<const uint8_t*>(str);
}

inline constexpr uint32_t decode2(const uint8_t* mb2) noexcept
{
	return static_cast<char32_t>( ( (mb2[0] & B2_MASK) << SH2) + tail(mb2[1]) );
}

inline constexpr char32_t decode3(const uint8_t* mb3) noexcept
{
	return static_cast<char32_t>( ( (mb3[0] & B3_MASK) << SH3)  +
		   ( tail(mb3[1]) << SH2) +
		   ( tail(mb3[2]) ) );
}

inline constexpr char32_t decode4(const uint8_t* mb4) noexcept
{
	return static_cast<char32_t>( ( (mb4[0] & B4_MASK) << SH4) +
		   ( tail(mb4[1]) << SH3) +
		   ( tail(mb4[2]) << SH2) +
		   tail(mb4[3]) );
}

}

/// Checks a byte is UTF-8 single byte character
inline constexpr bool isonebyte(const char c)
{
	return static_cast<uint8_t>(c) < uint8_t(0x80U);
}

/// Checks a byte is UTF-8 character tail byte
inline constexpr bool ismbtail(const char c) noexcept
{
	return 2 == ( uint8_t(c) >> 6);
}

/// Returns UTF-8 character size in bytes, based on first UTF-8 byte
inline constexpr unsigned int char_size(const char ch)
{
	return io_likely( isonebyte(ch) )
	? 1
	:
	// bit scan forward on inverted value gives number of leading multibyte bits
	// works much faster then mask check series on CPU which supports clz or bt instruction
	// (most of CPU supporting it)
#ifdef IO_IS_LITTLE_ENDIAN
	static_cast<unsigned int>( io_clz( ~( static_cast<unsigned int>(ch) << detail::MBSHIFT  ) ) );
#else
	static_cast<unsigned int>( io_clz( ~ ( static_cast<unsigned int>(ch) ) ) );
#endif // IO_IS_LITTLE_ENDIAN
}

/// Returns whether symbol if first byte of UTF-8 multibyte character
/// \return true when c points on UTF-8 mutibyte character first byte, otherwise false
inline constexpr bool ismblead(const char c)
{
	return char_size(c) > 2;
}

#ifdef IO_IS_LITTLE_ENDIAN

using detail::decode2;
using detail::decode3;
using detail::decode4;

#else

// invert byte ordering in case of big endian

inline constexpr char32_t decode2(const uint8_t* mb2) noexcept
{
	return io_bswap32( detail::decode2(mb2) );
}

inline constexpr char32_t decode3(const uint8_t* mb3) noexcept
{
	return io_bswap32( detail::decode3(mb3) );
}

inline constexpr char32_t decode4(const uint8_t* mb4) noexcept
{
	return io_bswap32( detail::decode4(mb4) );
}

#endif // IO_IS_LITTLE_ENDIAN

inline constexpr uint32_t decode2(const char* mb2) noexcept
{
	return decode2( detail::make_unsigned(mb2) );
}

inline constexpr char32_t decode3(const char* mb3) noexcept
{
	return decode3( detail::make_unsigned(mb3) );
}

inline constexpr char32_t decode4(const char* mb4) noexcept
{
	return decode3( detail::make_unsigned(mb4) );
}

/// Converts a UTF-8 single/multibyte character to full UNICODE UTF-32 value,
/// endianes depends on current CPU
/// \param dst destination UTF-32 character, or U'\0' when end of line reached or invalid source character value
/// \param src pointer to the UTF-8 character value
/// \return string position after src UTF-8 or nullptr when end of line reached or decoding failed
inline const char* mbtochar32(char32_t& dst, const char* src)
{
	switch( char_size(*src) ) {
	case 1:
		dst = static_cast<char32_t>( *src );
		return U'0' != dst ? src + 1: nullptr;
	case 2:
		dst = decode2( src );
		return src + 2;
	case 3:
		dst = decode3( src );
		return src + 3;
	case 4:
		dst = decode4( src );
		return src + 4;
	default:
		break;
	}
	dst = U'\0';
	return nullptr;
}


/// Returns UTF-8 string length in logical UNICODE characters
/// \param u8str source UTF-8 string
/// \return length in characters
#if defined(__GNUG__) && (__cplusplus > 201402)
constexpr std::size_t strlength(const char* u8str) noexcept {
#else
inline std::size_t strlength(const char* u8str) noexcept {
#endif
	std::size_t ret = 0;
	for(const char *c = u8str; '\0' != *c; c += char_size(*c) )
		++ret;
	return ret;
}


} // namespace u8

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
	not_supported,

	/// An unknown error
	unknown = -1
};

class IO_PUBLIC_SYMBOL chconv_error_category final: public std::error_category {
private:
#ifndef  _MSC_VER
	friend std::error_code  make_error_code(io::converrc ec) noexcept;
	friend std::error_condition  make_error_condition(io::converrc err) noexcept;
#else
	friend IO_PUBLIC_SYMBOL std::error_code make_error_code(io::converrc errc) noexcept;
	friend IO_PUBLIC_SYMBOL std::error_condition make_error_condition(io::converrc err) noexcept;
#endif
	static inline const chconv_error_category* instance()
	{
		static chconv_error_category _instance;
		return &_instance;
	}

	const char* cstr_message(int err_code) const;

public:

	constexpr chconv_error_category() noexcept:
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

#ifdef _MSC_VER
	IO_PUBLIC_SYMBOL std::error_code make_error_code(io::converrc errc) noexcept;
	IO_PUBLIC_SYMBOL std::error_condition make_error_condition(io::converrc err) noexcept;
#else
	std::error_code IO_PUBLIC_SYMBOL make_error_code(io::converrc errc) noexcept;
	std::error_condition  IO_PUBLIC_SYMBOL make_error_condition(io::converrc err) noexcept;
#endif

enum class cnvrt_control
{
	failure_on_failing_chars,
	discard_on_failing_chars
};


namespace detail {

class engine {
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

	converrc convert(uint8_t** src,std::size_t& size, uint8_t** dst, std::size_t& avail) const noexcept;
private:
	iconv_t iconv_;
};

} // namespace detail

class IO_PUBLIC_SYMBOL code_cnvtr;
DECLARE_IPTR(code_cnvtr);

/// \brief character set conversation (transcoding) interface
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
	void convert(std::error_code& ec, uint8_t** in,std::size_t& in_bytes_left,uint8_t** const out, std::size_t& out_bytes_left) const noexcept;

	void convert(std::error_code& ec, const uint8_t* src,const std::size_t src_size, byte_buffer& dst) const noexcept;

private:
	code_cnvtr(detail::engine&& eng) noexcept;
	friend class io::nobadalloc<code_cnvtr>;
public:
	static s_code_cnvtr open(std::error_code& ec,const charset& from,const charset& to, cnvrt_control conrol) noexcept;
	static std::error_condition _ok;
private:
	detail::engine eng_; // converting engine
};

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

} // namespace io

namespace std {

template<>
struct is_error_condition_enum<io::converrc> : public true_type
{};

} // namespace std

#endif // __IO_CHARSETS_CVT_HPP_INCLUDED__
