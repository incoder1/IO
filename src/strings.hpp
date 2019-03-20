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

typedef std::char_traits<char> char8_traits;

/// Compares two character code points
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

inline const char* find_delimiter(const char* where, const char* delimitters) noexcept {
	return io_strpbrk( where, delimitters);
}

// [A-Z] or [a-z] or [0-9] etc
inline constexpr bool between(uint32_t first, uint32_t last, uint32_t ch) noexcept
{
	return ( ch >= first ) && ( ch <= last  );
}

inline bool is_alpha(const char ch) noexcept
{
	return 0 != io_isalpha( ch );
}


// \s [^ \t\r\n\v\f]
inline bool is_whitespace(const char ch) noexcept
{
	return io_isspace( ch );
}

inline bool is_alnum(const char ch) noexcept
{
	return is_alpha(ch) || io_isdigit(ch);
}

// capital  unicode-ASCII-latin1 [A-Z]
static constexpr inline bool is_uppercase_latin1(const char ch) noexcept
{
	return io_isupper( ch );
}

// unicode-ASCII-latin1 [a-z]
static constexpr inline bool is_lowercase_latin1(const char ch) noexcept
{
	return io_islower( ch );
}

// [A-Z,a-z]
static inline bool is_latin1(const char ch) noexcept
{
	return io_isalpha( ch );
}

inline char latin1_to_lower(const char ch) noexcept
{
	return io_tolower(ch);
}

inline char latin1_to_upper(const char ch) noexcept
{
	return io_toupper(ch);
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
	return nullptr != strchrn(chars, len, what);
}

inline bool is_not_one(char what, const char* chars,const std::size_t len) noexcept {
	return nullptr == char8_traits::find(chars, len, what);
}

inline char* find_first_symbol(const char* s) noexcept
{
	return const_cast<char*>(s) + io_strspn(s, "\t\n\v\f\r ");
}

inline size_t xmlname_strspn(const char *s) noexcept
{
	return io_strcspn( s, "\t\n\v\f\r />");
}


} // namespace io

#endif // __IO_STRINGS_HPP_INCLUDED__
