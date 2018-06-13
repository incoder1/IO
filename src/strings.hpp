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
#include <cstring>
#include <climits>

#include <initializer_list>
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
constexpr inline bool cheq(c_1 lsh, c_2 rhs)
{
	typedef std::char_traits<c_1> c1tr;
	typedef std::char_traits<c_2> c2tr;
	return uint32_t( c1tr::to_int_type(lsh) ) == uint32_t( c2tr::to_int_type(rhs) );
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

template<typename _cht1, typename _cht2>
constexpr inline bool is_one_of(_cht1 what, _cht2 c1, _cht2 c2, _cht2 c3, _cht2 c4)
{
	return is_one_of(what,c1,c2,c3) || cheq(what, c4);
}

template<typename _cht1, typename _cht2>
constexpr inline bool is_one_of(_cht1 what, _cht2 c1, _cht2 c2, _cht2 c3, _cht2 c4, _cht2 c5)
{
	return is_one_of(what,c1,c2,c3,c4) || cheq(what, c5);
}

template<typename _cht1, typename _cht2>
constexpr inline bool is_one_of(_cht1 what, _cht2 c1, _cht2 c2, _cht2 c3, _cht2 c4, _cht2 c5, _cht2 c6)
{
	return is_one_of(what,c1,c2,c3,c4,c5) || cheq(what, c6);
}

template<typename _cht1, typename _cht2>
constexpr inline bool is_one_of(_cht1 what, _cht2 c1, _cht2 c2, _cht2 c3, _cht2 c4, _cht2 c5, _cht2 c6, _cht2 c7)
{
	return is_one_of(what,c1,c2,c3,c4,c5,c6) || cheq(what, c7);
}

template<typename _cht1, typename _cht2>
constexpr inline bool is_one_of(_cht1 what, _cht2 c1, _cht2 c2, _cht2 c3, _cht2 c4, _cht2 c5, _cht2 c6, _cht2 c7, _cht2 c8)
{
	return is_one_of(what,c1,c2,c3,c4,c5,c6,c7) || cheq(what, c8);
}

template<typename _cht1, typename _cht2>
inline bool is_one_of(_cht1 what, std::initializer_list<_cht2> spn)
{
	for(auto it = spn.begin(); it != spn.end(); ++it)
		if( cheq(what, *it) )
			return true;
	return false;
}

inline const char* find_delimiter(const char* where, const char* delimitters)
{
#ifdef  io_strpbrk
	return io_strpbrk( where, delimitters);
#else
    return std::strpbrk( where, delimitters);
#endif // io_strcspn
}

// [A-Z] or [a-z] or [0-9] etc
inline constexpr bool between(uint32_t first, uint32_t last, uint32_t ch)
{
	return ( ch <= last  ) && ( ch >= first );
}

template<typename __char_t>
inline constexpr bool is_alpha(__char_t ch)
{
    typedef std::char_traits<__char_t> chtr;
#ifdef io_isalpha
		return 0 != io_isalpha( chtr::to_int_type(ch) );
#else
	return between(
			char8_traits::to_int_type('a'),
			char8_traits::to_int_type('z'),
			chtr::to_int_type(ch) )
		  ||
		   between(
			 char8_traits::to_int_type('A'),
			 char8_traits::to_int_type('Z'),
			 chtr::to_int_type(ch));
#endif // io_isalpha
}


// \s [^ \t\r\n\v\f]
template<typename __char_t>
inline constexpr bool is_whitespace(__char_t ch)
{
	typedef std::char_traits<__char_t> tr;
#ifdef io_isspace
	return io_isspace( tr::to_int_type(ch) );
#else
	return char8_traits::to_int_type(' ') == tr::to_int_type(ch)
	       || between(
				char8_traits::to_int_type('\t'),
				char8_traits::to_int_type('\r'),
				tr::to_int_type(ch)
			  );
#endif // io_isspace
}


// capital  unicode-ASCII-latin1
template<typename _char_t>
constexpr inline bool is_digit(_char_t ch)
{
	typedef std::char_traits<_char_t> tr;
#ifdef io_isdigit
	return io_isdigit( tr::to_int_type(ch) );
#else
	return between(
			char8_traits::to_int_type('0'),
			char8_traits::to_int_type('9'),
			tr::to_int_type(ch)
		);
#endif // io_isdigit
}

template<typename __char_t>
inline constexpr bool is_alnum(__char_t ch)
{
	return is_alpha(ch) || is_digit(ch);
}


// capital  unicode-ASCII-latin1 [A-Z]
template<typename _char_t>
static constexpr inline bool is_uppercase_latin1(_char_t ch)
{
	typedef std::char_traits<_char_t> tr;
#ifdef io_isupper
	return io_isupper(tr::to_int_type(ch));
#else
	return between(
				char8_traits::to_int_type('A'),
				char8_traits::to_int_type('Z'),
				tr::to_int_type(ch)
			);
#endif // io_isupper
}

// unicode-ASCII-latin1 [a-z]
template<typename _char_t>
static constexpr inline bool is_lowercase_latin1(_char_t ch) noexcept
{
#ifdef io_islower
	typedef std::char_traits<_char_t> tr;
	return io_islower(tr::to_int_type(ch));
#else
	return between( char8_traits::to_int_type('a'), char8_traits::to_int_type('z'), ch );
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
#ifdef io_memcmp
	return 0 == io_memcmp( static_cast<const void*>(s), static_cast<const void*>(pattern), size);
#else
	return 0 == char8_traits::compare(s, pattern, size );
#endif // io_strcmp
}

inline bool is_xml_name_start_char_lo(char32_t ch) noexcept
{
	// _ | :
	return is_one_of(ch,0x5F,0x3A) || is_latin1(ch);
}

// Works only for UCS-4
inline bool is_xml_name_start_char(char32_t ch) noexcept
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
static constexpr inline bool is_xml_name_char(char32_t ch) noexcept
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
	return memory_traits::distance(b, e);
}

static constexpr const size_t ALIGN = sizeof(size_t)-1;
static constexpr const size_t ONES = size_t(~0) / uint8_t(~0);
static constexpr size_t HIGHS = ( ONES * ( (uint8_t(~0)/2) + 1 ) );

static constexpr inline bool no_zerro(const size_t x)
{
	return 0 == ( (x-ONES) & ~x & HIGHS);
}

#ifdef io_strchr
inline char* tstrchr(const char* s,char c)
{
	return io_strchr( const_cast<char*>(s), char8_traits::to_int_type(c) );
}
#else

static char* IO_NO_INLINE strchr_impl(const char* s,char c) noexcept
{
	const char *a = s;
	const uint8_t uc = static_cast<uint8_t>(c);
	if ( cheq('\0',c) )
		return const_cast<char*>( s + io_strlen(s) );
	while(reinterpret_cast<std::size_t>(s) % ALIGN) {
		if( cheq('\0',*s) )
			return nullptr;
		else if ( cheq(*s,uc) )
			return const_cast<char*>(s);
		++s;
	}
	const std::size_t *w = reinterpret_cast<const std::size_t*>(s);
	std::size_t k = ONES * uc;
	while( no_zerro(*w) && no_zerro(*w^k) )
		++w;
	s = reinterpret_cast<const char*>(w);
	while( !cheq(*s, uc) ) {
		if( cheq('\0',*s) )
			return nullptr;
		++s;
	}
	return a != s ? const_cast<char*>(s) : nullptr;
}

inline char* tstrchr(const char* s,char c)
{
	return strchr_impl(s,c);
}

#endif // io_strchr

inline bool is_one_of(char what, const char* chars)
{
	return nullptr != tstrchr(chars, what);
}


inline char* tstrchrn(const char* s,char c,std::size_t max_len)
{
	return static_cast<char*>( const_cast<void*>( io_memchr( s, static_cast<int>(c), max_len) ) );
}

template<typename _char_t>
inline std::size_t tstrchrn(const _char_t *str, _char_t character, std::size_t max_len)
{
	typedef std::char_traits<_char_t> traits_t;
	return traits_t::find(str, max_len, character);
}

template<typename _char_t>
inline std::size_t tstrchr(const _char_t *str, _char_t character)
{
	typedef std::char_traits<_char_t> traits_t;
	return traits_t::find(str, traits_t::length(str), character);
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
		if( io_unlikely( cheq('\0',*us) ) )
			return nullptr;
		sw = (sw << 8) | uint16_t(*us);
	}
	return reinterpret_cast<const char*>( us-1 );
}

inline char* find_first_symbol(const char* s) {
	char* ret = const_cast<char*>(s);
	while( is_whitespace(*ret) )
		++ret;
	return ret;
}

inline size_t xmlname_strspn(const char *s)
{
	static constexpr const char* pattern = "\t\n\v\f\r />";
	return io_strcspn( s, pattern);
}

inline constexpr bool single_byte(const char c) {
	return static_cast<uint8_t>(c) < uint8_t(0x80U);
}

inline constexpr bool ismbnext(const char c) {
	return uint8_t(0xBFU) == uint8_t(c | 0xBFU);
}

inline uint8_t u8_char_size(const char ch) {
	if( io_likely( single_byte(ch) ) )
		return 1;
#ifdef IO_IS_LITTLE_ENDIAN
		static constexpr unsigned int MB_SHIFT = ( sizeof(unsigned int) << 3 ) - 8;
		unsigned int c = static_cast<unsigned int>(ch) << MB_SHIFT;
#else
		unsigned int c = static_cast<unsigned int>(ch);
#endif // IO_IS_LITTLE_ENDIAN
		return static_cast<unsigned int>( io_clz( ~c ) );
}


} // namespace io


#endif // __IO_STRINGS_HPP_INCLUDED__
