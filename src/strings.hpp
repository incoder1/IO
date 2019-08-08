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

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

namespace io {

typedef std::char_traits<char> char8_traits;

/// Compares two character code points
template<typename c_1, typename c_2>
constexpr bool cheq(c_1 lhs, c_2 rhs) noexcept
{
	return static_cast<unsigned int>(lhs) == static_cast<unsigned int>(rhs);
}

constexpr bool cheq(char lhs, char rhs) noexcept
{
	return lhs == rhs;
}

//  [<0>...<1>]
template<typename _cht1, typename _cht2>
constexpr bool is_one_of(_cht1 what, _cht2 c1, _cht2 c2)
{
	return cheq(what,c1) || cheq(what, c2);
}

//  [<0>...<2>]
template<typename _cht1, typename _cht2>
constexpr bool is_one_of(_cht1 what, _cht2 c1, _cht2 c2, _cht2 c3)
{
	return is_one_of(what,c1,c2) || cheq(what, c3);
}

template<typename _cht1, typename _cht2>
constexpr bool is_one_of(_cht1 what, _cht2 c1, _cht2 c2, _cht2 c3, _cht2 c4)
{
	return is_one_of(what,c1,c2,c3) || cheq(what, c4);
}

template<typename _cht1, typename _cht2>
constexpr bool is_one_of(_cht1 what, _cht2 c1, _cht2 c2, _cht2 c3, _cht2 c4, _cht2 c5)
{
	return is_one_of(what,c1,c2,c3,c4) || cheq(what, c5);
}

template<typename _cht1, typename _cht2>
constexpr bool is_one_of(_cht1 what, _cht2 c1, _cht2 c2, _cht2 c3, _cht2 c4, _cht2 c5, _cht2 c6)
{
	return is_one_of(what,c1,c2,c3,c4,c5) || cheq(what, c6);
}

template<typename _cht1, typename _cht2>
constexpr bool is_one_of(_cht1 what, _cht2 c1, _cht2 c2, _cht2 c3, _cht2 c4, _cht2 c5, _cht2 c6, _cht2 c7)
{
	return is_one_of(what,c1,c2,c3,c4,c5,c6) || cheq(what, c7);
}

template<typename _cht1, typename _cht2>
constexpr bool is_one_of(_cht1 what, _cht2 c1, _cht2 c2, _cht2 c3, _cht2 c4, _cht2 c5, _cht2 c6, _cht2 c7, _cht2 c8)
{
	return is_one_of(what,c1,c2,c3,c4,c5,c6,c7) || cheq(what, c8);
}

inline const char* find_delimiter(const char* where, const char* delimitters) noexcept {
	return io_strpbrk( where, delimitters);
}

// [A-Z] or [a-z] or [0-9] etc
//constexpr bool between(uint32_t first, uint32_t last, uint32_t ch) noexcept
//{
//	return ( ch >= first ) && ( ch <= last  );
//}

static constexpr const unsigned int BITS = sizeof(unsigned int) * CHAR_BIT;
static constexpr const unsigned int ALBT_SIZE = 26;
static constexpr const unsigned int LAT_A = static_cast<unsigned int>('a');
static constexpr const unsigned int LAT_A_C = static_cast<unsigned int>('A');
static constexpr const unsigned int ZERRO = static_cast<unsigned int>('0');
static constexpr const char UPPER_LOWER_MASK = 0x5F;

constexpr bool is_alpha(const char ch) noexcept
{
	return ( (static_cast<unsigned int>(ch) | BITS ) - LAT_A) < ALBT_SIZE;
}

constexpr bool is_digit(const char ch) noexcept {
	return (static_cast<unsigned int>(ch)-ZERRO) < 10;
}

constexpr bool is_alnum(const char ch) noexcept
{
	return is_alpha(ch) || is_digit(ch);
}

// capital  unicode-ASCII-latin1 [A-Z]
static constexpr bool is_uppercase_latin1(const char ch) noexcept
{
	return  (static_cast<unsigned int>(ch)-LAT_A_C) < ALBT_SIZE;
}

// unicode-ASCII-latin1 [a-z]
static constexpr bool is_lowercase_latin1(const char ch) noexcept
{
	return (static_cast<unsigned int>(ch)-LAT_A) < ALBT_SIZE;
}

// [A-Z,a-z]
static constexpr bool is_latin1(const char ch) noexcept
{
	return is_lowercase_latin1( ch ) || is_uppercase_latin1( ch );
}

constexpr char latin1_to_lower(const char ch) noexcept
{
	return is_uppercase_latin1( ch ) ? (ch & UPPER_LOWER_MASK) : ch;
}

constexpr char latin1_to_upper(const char ch) noexcept
{
	return is_uppercase_latin1( ch ) ? (ch & UPPER_LOWER_MASK) : ch;
}

inline bool start_with(const char* s,const char* pattern,const std::size_t size) noexcept
{
	return 0 == io_memcmp( static_cast<const void*>(s), static_cast<const void*>(pattern), size);
}

inline std::size_t str_size(const char* b, const char* e) noexcept
{
	return memory_traits::distance(b, e);
}

inline char* strchrn(const char* s,const char c,const std::size_t max_len) noexcept
{
	return const_cast<char*>( char8_traits::find(s, max_len, c) );
}

inline bool is_one_of(char what, const char* chars,const std::size_t len) noexcept
{
	return nullptr != strchrn(chars, what, len);
}

inline char* find_first_symbol(const char* s) noexcept
{
	constexpr const char* sym = "\t\n\v\f\r ";
	return const_cast<char*>(s) + io_strspn(s, sym);
}

inline size_t xmlname_strspn(const char *s) noexcept
{
	constexpr const char* sym = "\t\n\v\f\r />";
	return io_strcspn(s, sym);
}

constexpr uint16_t pack_word(uint16_t w, char c) noexcept {
#ifdef IO_IS_LITTLE_ENDIAN
	return (w << CHAR_BIT) | static_cast<uint16_t>(c);
#else
	return (w >> CHAR_BIT) | static_cast<uint16_t>(ch);
#endif // IO_IS_LITTLE_ENDIAN
}

} // namespace io

#endif // __IO_STRINGS_HPP_INCLUDED__
