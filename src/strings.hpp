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
#ifndef __IO_STRINGS_HPP_INCLUDED__
#define __IO_STRINGS_HPP_INCLUDED__

#include "config.hpp"

#include "charsetcvt.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

namespace io
{

namespace detail
{

template<typename char_t>
constexpr unsigned int unsign(const char_t ch) noexcept
{
    return static_cast<unsigned int>( std::char_traits<char_t>::to_int_type(ch) );
}

constexpr unsigned int unsign(const int ch) noexcept
{
    return static_cast<unsigned int>( ch );
}

} // namespace detail

/// Compares two character code points
template<typename lhs_ch_t, typename rhs_ch_t>
constexpr bool cheq(const lhs_ch_t lhs,const rhs_ch_t rhs) noexcept
{
    return detail::unsign(lhs) == detail::unsign(rhs);
}

/// Compares two character code points
template<typename lhs_ch_t, typename rhs_ch_t>
constexpr bool chnoteq(const lhs_ch_t lhs,const rhs_ch_t rhs) noexcept
{
    return detail::unsign(lhs) != detail::unsign(rhs);
}

constexpr bool cheq(char lhs, char rhs) noexcept
{
    return lhs == rhs;
}

constexpr bool chnoteq(char lhs, char rhs) noexcept
{
    return lhs != rhs;
}

template<typename __CharType, __CharType __A, __CharType __B>
constexpr bool is_one_of(__CharType c) noexcept
{
    return cheq(__A,c) || cheq(__B,c);
}

template<typename __CharType, __CharType __A, __CharType __B>
constexpr bool is_none_of(__CharType c) noexcept
{
    return chnoteq(__A, c) && chnoteq(__B, c);
}


template<typename __CharType, __CharType __A,__CharType __B, __CharType __C>
constexpr bool is_one_of(__CharType c) noexcept
{
    return cheq(__A, c) || cheq(__B, c) || cheq(__C, c);
}

template<typename __CharType, __CharType __A, __CharType __B, __CharType __C>
constexpr bool is_none_of(__CharType c) noexcept
{
    return chnoteq(__A, c) && chnoteq(__B, c) && chnoteq(__C, c);
}

template<typename __CharType, __CharType __A,__CharType __B,__CharType __C,__CharType __D>
constexpr bool is_one_of(__CharType c) noexcept
{
    return cheq(__A, c) || cheq(__B, c) || cheq(__C, c) || cheq(__D, c);
}

template<typename __CharType, __CharType __A,__CharType __B,__CharType __C,__CharType __D>
constexpr bool is_none_of(__CharType c) noexcept
{
    return chnoteq(__A, c) && chnoteq(__B, c) && chnoteq(__C, c) && chnoteq(__D, c);
}

// char
template<char __A, char __B>
constexpr bool is_one_of(char c) noexcept
{
    return is_one_of<char, __A, __B>(c);
}

template<char __A, char __B>
constexpr bool is_none_of(char c) noexcept
{
    return is_none_of<char, __A, __B>(c);
}

template<char __A, char __B, char __C>
constexpr bool is_one_of(char c) noexcept
{
    return is_one_of<char, __A, __B, __C>(c);
}

template<char __A, char __B, char __C>
constexpr bool is_none_of(char c) noexcept
{
    return is_none_of<char, __A, __B, __C>(c);
}


template<char __A, char __B, char __C, char __D>
constexpr bool is_one_of(char c) noexcept
{
    return is_one_of<char, __A, __B, __C, __D>(c);
}

template<char __A, char __B, char __C, char __D>
constexpr bool is_none_of(char c) noexcept
{
    return is_none_of<char, __A, __B, __C, __D>(c);
}

// wchar_t
template<wchar_t __A, wchar_t __B>
constexpr bool is_one_of(wchar_t c) noexcept
{
    return is_one_of<wchar_t, __A, __B>(c);
}

template<wchar_t __A, wchar_t __B>
constexpr bool is_none_of(wchar_t c) noexcept
{
    return is_none_of<wchar_t, __A, __B>(c);
}

template<wchar_t __A, wchar_t __B, wchar_t __C>
constexpr bool is_one_of(wchar_t c) noexcept
{
    return is_one_of<wchar_t, __A, __B, __C>(c);
}

template<wchar_t __A, wchar_t __B, wchar_t __C>
constexpr bool is_none_of(wchar_t c) noexcept
{
    return is_none_of<wchar_t, __A, __B, __C>(c);
}

template<wchar_t __A, wchar_t __B, wchar_t __C, wchar_t __D>
constexpr bool is_one_of(wchar_t c) noexcept
{
    return is_one_of<wchar_t, __A, __B, __C, __D>(c);
}

template<wchar_t __A, wchar_t __B, wchar_t __C, wchar_t __D>
constexpr bool is_none_of(wchar_t c) noexcept
{
    return is_none_of<wchar_t, __A, __B, __C, __D>(c);
}

// char16_t
template<char16_t __A, char16_t __B>
constexpr bool is_one_of(char16_t c) noexcept
{
    return is_one_of<char16_t, __A, __B>(c);
}

template<char16_t __A, char16_t __B>
constexpr bool is_none_of(char16_t c) noexcept
{
    return is_none_of<char16_t, __A, __B>(c);
}

template<char16_t __A, char16_t __B, char16_t __C>
constexpr bool is_one_of(char16_t c) noexcept
{
    return is_one_of<char16_t, __A, __B, __C>(c);
}

template<char16_t __A, char16_t __B, char16_t __C>
constexpr bool is_none_of(char16_t c) noexcept
{
    return is_none_of<char16_t, __A, __B, __C>(c);
}

template<char16_t __A, char16_t __B, char16_t __C, char16_t __D>
constexpr bool is_one_of(char16_t c) noexcept
{
    return is_one_of<char16_t, __A, __B, __C, __D>(c);
}

template<char16_t __A, char16_t __B, char16_t __C, char16_t __D>
constexpr bool is_none_of(char16_t c) noexcept
{
    return is_none_of<char16_t, __A, __B, __C, __D>(c);
}

// char32_t
template<char32_t __A, char32_t __B>
constexpr bool is_one_of(char32_t c) noexcept
{
    return is_one_of<char32_t, __A, __B>(c);
}

template<char32_t __A, char32_t __B>
constexpr bool is_none_of(char32_t c) noexcept
{
    return is_none_of<char32_t, __A, __B>(c);
}

template<char32_t __A, char32_t __B, char32_t __C>
constexpr bool is_one_of(char32_t c) noexcept
{
    return is_one_of<char32_t, __A, __B, __C>(c);
}

template<char32_t __A, char32_t __B, char32_t __C>
constexpr bool is_none_of(char32_t c) noexcept
{
    return is_none_of<char32_t, __A, __B, __C>(c);
}

template<char32_t __A, char32_t __B, char32_t __C, char32_t __D>
constexpr bool is_one_of(char32_t c) noexcept
{
    return is_one_of<char32_t, __A, __B, __C, __D>(c);
}

template<char32_t __A, char32_t __B, char32_t __C, char32_t __D>
constexpr bool is_none_of(char32_t c) noexcept
{
    return is_none_of<char32_t, __A, __B, __C, __D>(c);
}

static constexpr const unsigned int BITS = sizeof(unsigned int) * CHAR_BIT;
static constexpr const unsigned int ALBT_SIZE = 26;
static constexpr const unsigned int LAT_A = detail::unsign('a');
static constexpr const unsigned int LAT_A_CAP = detail::unsign('A');
static constexpr const unsigned int ZERRO = detail::unsign('0');
static constexpr const unsigned int SPACE = detail::unsign(' ');
static constexpr const unsigned int TAB = detail::unsign('\t');
static constexpr const unsigned int WHITE_SPACES_COUNT = 5;

static constexpr const char UPPER_LOWER_MASK = 0x5F;

template<typename char_t>
constexpr bool is_alpha(const char_t ch) noexcept
{
    return ( (detail::unsign(ch) | BITS ) - LAT_A) < ALBT_SIZE;
}

template<typename char_t>
constexpr bool is_digit(const char_t ch) noexcept
{
    return (detail::unsign(ch)-ZERRO) < 10;
}

template<typename char_t>
constexpr bool is_alnum(const char_t ch) noexcept
{
    return is_alpha(ch) || is_digit(ch);
}

template<typename char_t>
constexpr bool is_space(const char_t ch) noexcept
{
    return SPACE == detail::unsign(ch) || (detail::unsign(ch) - TAB) < WHITE_SPACES_COUNT;
}

// capital  unicode-ASCII-latin1 [A-Z]
template<typename char_t>
static constexpr bool is_uppercase_latin1(const char_t ch) noexcept
{
    return  (detail::unsign(ch)-LAT_A_CAP) < ALBT_SIZE;
}

// unicode-ASCII-latin1 [a-z]
template<typename char_t>
static constexpr bool is_lowercase_latin1(const char_t ch) noexcept
{
    return (detail::unsign(ch)-LAT_A) < ALBT_SIZE;
}

// [A-Z,a-z]
template<typename char_t>
constexpr bool is_latin1(const char_t ch) noexcept
{
    return is_lowercase_latin1( ch ) || is_uppercase_latin1( ch );
}

constexpr char latin1_to_lower(const char ch) noexcept
{
    return is_uppercase_latin1( ch ) ? (ch & UPPER_LOWER_MASK) : ch;
}

constexpr char latin1_to_upper(const char ch) noexcept
{
    return is_lowercase_latin1( ch ) ? (ch & UPPER_LOWER_MASK) : ch;
}

inline char* strchrn(const char* s,const char c,const std::size_t max_len) noexcept
{
    return const_cast<char*>( std::char_traits<char>::find(s, max_len, c) );
}

constexpr bool is_endl(const char ch) noexcept
{
    return '\0' == ch;
}

constexpr bool not_endl(const char ch) noexcept
{
    return '\0' != ch;
}

constexpr uint16_t pack_word(uint16_t w, char c) noexcept
{
#ifdef IO_IS_LITTLE_ENDIAN
    return (w << CHAR_BIT) | static_cast<uint16_t>(c);
#else
    return (w >> CHAR_BIT) | static_cast<uint16_t>(c);
#endif // IO_IS_LITTLE_ENDIAN
}

} // namespace io

#endif // __IO_STRINGS_HPP_INCLUDED__
