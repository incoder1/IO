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
#ifndef __IO_STRINGS_HPP_INCLUDED__
#define __IO_STRINGS_HPP_INCLUDED__

#include "config.hpp"

#include "charsetcvt.hpp"
//
#include <string>

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

namespace io {

namespace {
	typedef std::char_traits<char> char8_traits;
}

/// Compares two characters
template<typename c_1, typename c_2>
constexpr inline bool cheq(c_1 lsh, c_2 rhs )
{
	typedef std::char_traits<c_1> c1tr;
	typedef std::char_traits<c_2> c2tr;
	return uint_fast32_t(c1tr::to_int_type(lsh)) == uint_fast32_t(c2tr::to_int_type(rhs));
}

static constexpr inline bool cheq(char lsh, char rhs)
{
	return lsh == rhs;
}

//  [<0>...<1>]
template<typename _cht1, typename _cht2>
constexpr inline bool is_one_of(_cht1 what, _cht2 c1, _cht2 c2)
{
	return cheq(what,c1) || cheq(what, c2);
}

//  [<0>...<2>]
template<typename _cht1, typename _cht2>
constexpr inline bool is_one_of(_cht1 what, _cht2 c1, _cht2 c2, _cht2 c3)
{
	return is_one_of(what,c1,c2) || cheq(what, c3);
}

// [A-Z] or [a-z] or [0-9] etc
inline constexpr bool between(uint_fast32_t first, uint_fast32_t last, uint_fast32_t ch)
{
	return ( ch <= last  ) && ( ch >= first );
}

// \s [^ \t\r\n\v\f]
template<typename __char_t>
inline constexpr bool is_whitespace(__char_t ch)
{
	typedef std::char_traits<__char_t> tr;
	return char8_traits::to_int_type(' ') == tr::to_int_type(ch)
	       || between( '\t', '\r', tr::to_int_type(ch) );
}


// capital  unicode-ASCII-latin1
template<typename _char_t>
constexpr inline bool is_digit(_char_t ch)
{
	return between('0','9', ch);
}

// capital  unicode-ASCII-latin1 [A-Z]
template<typename _char_t>
static constexpr inline is_uppercase_latin1(_char_t ch)
{
#ifdef io_isupper
	return io_isupper(static_cast<int>(ch));
#else
	return between( 'A', 'Z', ch );
#endif // io_isupper
}

// unicode-ASCII-latin1 [a-z]
template<typename _char_t>
static constexpr inline bool is_lowercase_latin1(_char_t ch) noexcept
{
#ifdef io_islower
	return io_islower(static_cast<int>(ch));
#else
	return between( 'a', 'z', ch );
#endif // io_islower
}

// [A-Z,a-z]
template<typename _char_t>
static constexpr inline bool is_latin1(_char_t ch) noexcept
{
	return is_uppercase_latin1(ch) || is_lowercase_latin1(ch);
}

inline char latin1_to_lower(const char ch) noexcept
{
#ifdef io_tolower
	return io_tolower(ch);
#else
	return is_uppercase_latin1(ch) ? 'z' - ('Z' - ch) : ch;
#endif // io_tolower
}

inline char latin1_to_upper(const char ch) noexcept
{
#ifdef io_toupper
	return io_toupper(ch);
#else
	return is_lowercase_latin1(ch) ? 'Z' - ('z' - ch) : ch;
#endif // io_toupper
}

inline bool start_with(const char* s,const char* pattern,std::size_t size) noexcept
{
	return 0 == char8_traits::compare(s, pattern, size );
}

inline constexpr bool is_xml_name_start_char_lo(uint32_t ch) noexcept
{
	// _ | :
	return is_one_of(ch,0x5F,0x3A) || is_latin1(ch);
}

// Works only for UCS-4
inline constexpr bool is_xml_name_start_char(uint_fast32_t ch) noexcept
{
	return is_xml_name_start_char_lo(ch) ||
	       between(0xC0,0xD6, ch)    ||
	       between(0xD8,0xF6, ch)    ||
	       between(0xF8,0x2FF,ch)    ||
	       between(0x370,0x37D,ch)   ||
	       between(0x37F,0x1FFF,ch)  ||
	       between(0x200C,0x200D,ch) ||
	       between(0x2070,0x218F,ch) ||
	       between(0x2C00,0x2FEF,ch) ||
	       between(0x3001,0xD7FF,ch) ||
	       between(0xF900,0xFDCF,ch) ||
	       between(0xFDF0,0xFFFD,ch) ||
	       between(0x10000,0xEFFFF,ch);
}

// Works only for UCS-4
static constexpr inline bool is_xml_name_char(uint_fast32_t ch) noexcept
{
	return is_digit(ch) ||
	       // - | . | U+00B7
	       is_one_of(ch,0x2D,0x2E,0xB7) ||
	       is_xml_name_start_char(ch) ||
	       between(0x0300,0x036F,ch)  ||
	       between(0x203F,0x2040,ch);
}

template<typename _char_t>
inline constexpr std::size_t str_size(const _char_t* b, const _char_t* e)
{
	return reinterpret_cast<std::size_t>(e) - reinterpret_cast<std::size_t>(b);
}

// search for character
template<typename _char_t>
inline constexpr _char_t* tstrchr(const _char_t* where,_char_t what)
{
	typedef std::char_traits<_char_t> tr;
	return nullptr == where || cheq(_char_t(0),*where)
	       ? nullptr : const_cast<_char_t*>( tr::find(where,  /*tr::length(where)*/ UINT_MAX, what) );
}


static constexpr const size_t ALIGN = sizeof(size_t)-1;
static constexpr const size_t ONES = size_t(~0) / uint8_t(~0);
static constexpr size_t HIGHS = ( ONES * ( (uint8_t(~0)/2) + 1 ) );

static constexpr inline bool no_zerro(const size_t x)
{
	return 0 == ( (x-ONES) & ~x & HIGHS);
}

inline char* tstrchr(const char* s,char c)
{
#ifdef io_strchr
	return io_strchr(s, static_cast<int>(c) );
#else
	const char *a = s;
	const uint8_t uc = static_cast<uint8_t>(c);
	if (uint8_t('\0') == c)
		return const_cast<char*>( s + io_strlen(s) );
	while(reinterpret_cast<std::size_t>(s) % ALIGN) {
		if('\0' == *s)
			return nullptr;
		else if (static_cast<uint8_t>(*s) == uc)
			return const_cast<char*>(s);
		++s;
	}
	const std::size_t *w = reinterpret_cast<const std::size_t*>(s);
	std::size_t k = ONES * uc;
	while( no_zerro(*w) && no_zerro(*w^k) )
		++w;
	s = reinterpret_cast<const char*>(w);
	while( static_cast<uint8_t>(*s) != uc) {
		if('\0' == *s)
			return nullptr;
		++s;
	}
	return a != s ? const_cast<char*>(s) : nullptr;
#endif // io_strchr
}

inline const char *strstr2b(const char *s, const char *n)
{
	const uint8_t *un = reinterpret_cast<const uint8_t*>(n);
	const uint16_t pw = (*un << 8) | *(un+1);
	const uint8_t *us = reinterpret_cast<const uint8_t*>(s);
	uint16_t sw = (*us << 8) | *(us+1);
	++us;
	while(pw != sw) {
		++us;
		if(uint8_t('\0') == *us)
			return nullptr;
		sw = (sw << 8) | uint16_t(*us);
	}
	return reinterpret_cast<const char*>( us-1 );
}

// search for first non whitespace
template<typename __char_t>
inline __char_t* find_first_symbol(const __char_t* s)
{
	typedef std::char_traits<__char_t> tr;
	static constexpr int TAB = char8_traits::to_int_type('\t');
	static constexpr int LF = char8_traits::to_int_type('\n');
	static constexpr int LT = char8_traits::to_int_type('\v');
	static constexpr int FF = char8_traits::to_int_type('\f');
	static constexpr int CR =  char8_traits::to_int_type('\r');
	static constexpr int SPACE =  char8_traits::to_int_type(' ');
	for(;;)  {
		switch( tr::to_int_type( *s ) ) {
		case 0:
			return nullptr;
		case TAB:
		case LF:
		case LT:
		case FF:
		case CR:
		case SPACE:
			++s;
			break;
		default:
			return const_cast<__char_t*>(s);
		}
	}
	return const_cast<__char_t*>(s);
}

// UTF-8 char mask helper
static constexpr inline bool check_mask(uint8_t mask, uint8_t value)
{
	return mask == (value & mask);
}

// UTF-8 double byte character
static constexpr inline bool is_u8_2(const char ch)
{
	return check_mask(0xC0,static_cast<uint8_t>(ch));
}

// UTF-8 triple byte character
static constexpr inline bool is_u8_3(const char ch)
{
	return check_mask(0xE0,static_cast<uint8_t>(ch));
}

// UTF-8 quadruple byte character
static constexpr inline bool is_u8_4(const char ch)
{
	return check_mask(0xF0,static_cast<uint8_t>(ch));
}

// UTF-8 non UNICODE space character
static constexpr inline bool is_u8_5_or_6(const char ch)
{
	return check_mask(0xF8,static_cast<uint8_t>(ch)) || check_mask(0xFC,static_cast<uint8_t>(ch));
}

// Gest UTF-8 character size
constexpr inline uint8_t u8_char_size(const char ch)
{
	return  (ch > 0) ? 1
	        :  is_u8_2(ch) ? 2
	        :  is_u8_3(ch) ? 3
	        :  is_u8_4(ch) ? 4
	        : 5;
}

} // namespace io


#endif // __IO_STRINGS_HPP_INCLUDED__
