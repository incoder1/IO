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
#ifndef __IO_UNICODE_BOM_HPP_INCLUDED__
#define __IO_UNICODE_BOM_HPP_INCLUDED__

#include <io/config/libio_config.hpp>

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <ostream>

#include <io/core/type_traits_ext.hpp>

#include "charsets.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

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

/// UNICODE UTF-8 byte order mark primitive
class IO_PUBLIC_SYMBOL utf8_bom  {
private:
	static const uint8_t* DATA;
public:
	static inline const uint8_t* data() noexcept {
		return DATA;
	}
	static constexpr inline std::size_t len() noexcept {
		return 3;
	}
	static inline bool is(const uint8_t* str) noexcept {
		return 0 == io_memcmp(DATA, str, len() );
	}
	static inline bool is(const char* str) {
		return is( reinterpret_cast<const uint8_t*>(str) );
	}
};

/// UNICODE UTF-16LE byte order mark primitive
class IO_PUBLIC_SYMBOL utf_16le_bom {
private:
	static const uint8_t *DATA;
public:
	static inline const uint8_t* data() noexcept {
		return DATA;
	}
	static constexpr inline std::size_t len() noexcept {
		return 2;
	}
	static inline bool is(const uint8_t* str) noexcept {
		return 0 == io_memcmp( DATA, str, len() );
	}
	static inline bool is(const char* str) {
		return is( reinterpret_cast<const uint8_t*>(str) );
	}
};

/// UNICODE UTF-32LE byte order mark primitive
class IO_PUBLIC_SYMBOL utf_32le_bom {
private:
	static const uint8_t *DATA;
public:
	static inline const uint8_t* data() noexcept {
		return DATA;
	}
	static constexpr inline std::size_t len() noexcept {
		return 4;
	}
	static inline bool is(const uint8_t* str) noexcept {
		return 0 == io_memcmp(DATA,str,len());
	}
	static inline bool is(const char* str) {
		return is( reinterpret_cast<const uint8_t*>(str) );
	}
};

/// UNICODE UTF-16BE byte order mark primitive
class IO_PUBLIC_SYMBOL utf_16be_bom {
private:
	static const uint8_t *DATA;
public:
	static inline const uint8_t* data() noexcept {
		return DATA;
	}
	static constexpr inline std::size_t len() noexcept {
		return 2;
	}
	static inline bool is(const uint8_t* str) noexcept {
		return 0 == io_memcmp(DATA,str,len());
	}
	static inline bool is(const char* str) {
		return is( reinterpret_cast<const uint8_t*>(str) );
	}
};

/// UNICODE UTF-32BE byte order mark primitive
class IO_PUBLIC_SYMBOL utf_32be_bom {
private:
	static const uint8_t *DATA;
public:
	static inline const uint8_t* data() noexcept {
		return DATA;
	}
	static constexpr inline std::size_t len() noexcept {
		return 4;
	}
	static inline bool is(const uint8_t* str) noexcept {
		return 0 == io_memcmp(DATA,str,len());
	}
	static inline bool is(const char* str) {
		return is( reinterpret_cast<const uint8_t*>(str) );
	}
};

/// UNICODE representation types enumeration
enum class unicode_cp {
	utf8,
	utf_16le,
	utf_16be,
	utf_32le,
	utf_32be,
	not_detected
};

template<typename _char_type,class _trailts_type = std::char_traits<_char_type> >
inline std::basic_ostream<_char_type,_trailts_type>& operator<<(std::basic_ostream<_char_type, _trailts_type>& os, unicode_cp bom) noexcept
{
	const uint8_t *bomd;
	std::size_t boml;
	switch(bom) {
	case unicode_cp::not_detected:
		return os;
	case unicode_cp::utf8:
		bomd = utf8_bom::data();
		boml = utf8_bom::len();
		break;
	case unicode_cp::utf_16le:
		bomd = utf_16le_bom::data();
		boml = utf_16le_bom::len();
		break;
	case unicode_cp::utf_32le:
		bomd = utf_32le_bom::data();
		boml = utf_32le_bom::len();
		break;
	case unicode_cp::utf_16be:
		bomd = utf_16be_bom::data();
		boml = utf_16be_bom::len();
		break;
	case unicode_cp::utf_32be:
		bomd = utf_32be_bom::data();
		boml = utf_32be_bom::len();
		break;
	}
	os.write( reinterpret_cast<const _char_type*>(bomd), boml);
	return os;
}

static inline unicode_cp detect_by_bom(const uint8_t * bom)
{
	if( utf8_bom::is(bom) )
		return unicode_cp::utf8;
	else if( utf_16le_bom::is(bom) ) {
		if( utf_32le_bom::is(bom) )
			return unicode_cp::utf_32le;
		else
			return unicode_cp::utf_16le;
	}
	else if( utf_16be_bom::is(bom) )
		return unicode_cp::utf_16be;
	else if( utf_32be_bom::is(bom) )
		return unicode_cp::utf_32be;
	return unicode_cp::not_detected;
}

} // namespace io


#endif // __IO_UNICODE_BOM_HPP_INCLUDED__
