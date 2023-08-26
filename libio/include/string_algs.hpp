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
#ifndef __IO_STRING_ALGS_HPP_INCLUDED__
#define __IO_STRING_ALGS_HPP_INCLUDED__

#include "config.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <string>
#include "type_traits_ext.hpp"

namespace io {

namespace detail {

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


constexpr bool cheq(unsigned char lhs,unsigned char rhs) noexcept
{
	return lhs == rhs;
}

constexpr bool chnoteq(unsigned char lhs,unsigned char rhs) noexcept
{
	return lhs != rhs;
}

/// Compares two character code points
/// \param lhs compare left hand statement code point
/// \param rhs compare right hand statement code point
/// \return whether code-points equals
#ifdef IO_HAS_CONNCEPTS
template<typename lhs_ch_t, typename rhs_ch_t>
	requires( is_charater_v<lhs_ch_t>, is_charater_v<rhs_ch_t> )
#else
template<
	typename lhs_ch_t,
	typename rhs_ch_t,
	typename std::enable_if<
		is_charater<lhs_ch_t>::value && is_charater<rhs_ch_t>::value
	>::type* = nullptr
>
#endif // IO_HAS_CONNCEPTS
constexpr bool cheq(const lhs_ch_t lhs,const rhs_ch_t rhs) noexcept
{
	return detail::unsign(lhs) == detail::unsign(rhs);
}

/// Check current symbol is enof of file symbol
/// \param c compare left hand statement code point
#ifdef IO_HAS_CONNCEPTS
template<typename char_type>
	requires(is_charater_v<char_type>)
#else
template<
	typename char_type,
	typename std::enable_if<
		is_charater<char_type>::value
	>::type* = nullptr
>
#endif // IO_HAS_CONNCEPTS
constexpr bool is_eof(char_type ch) noexcept {
	return std::char_traits<char_type>::eof() == ch;
}

/// Compares two character code points
/// \param lhs compare left hand statement code point
/// \param rhs compare right hand statement code point
/// \return whether code-points not equals
#ifdef IO_HAS_CONNCEPTS
template<typename lhs_ch_t, typename rhs_ch_t>
	requires( is_charater_v<lhs_ch_t>, is_charater_v<rhs_ch_t> )
#else
template<
	typename lhs_ch_t,
	typename rhs_ch_t,
	typename std::enable_if<
		is_charater<lhs_ch_t>::value && is_charater<rhs_ch_t>::value
	>::type* = nullptr
>
#endif // IO_HAS_CONNCEPTS
constexpr bool chnoteq(const lhs_ch_t lhs,const rhs_ch_t rhs) noexcept
{
	return detail::unsign(lhs) != detail::unsign(rhs);
}

#ifdef IO_HAS_CONNCEPTS
template<typename __CharType, __CharType __A, __CharType __B>
	requires( is_charater_v<__CharType> )
#else
template<
	typename __CharType, __CharType __A, __CharType __B,
	typename std::enable_if<
		is_charater<__CharType>::value
	>::type* = nullptr
>
#endif // IO_HAS_CONNCEPTS
constexpr bool is_one_of(__CharType c) noexcept
{
	return cheq(__A,c) || cheq(__B,c);
}

#ifdef IO_HAS_CONNCEPTS
template<typename __CharType, __CharType __A, __CharType __B>
	requires( is_charater_v<__CharType> )
#else
template<
	typename __CharType, __CharType __A, __CharType __B,
	typename std::enable_if<
		is_charater<__CharType>::value
	>::type* = nullptr
>
#endif // IO_HAS_CONNCEPTS
constexpr bool is_none_of(__CharType c) noexcept
{
	return chnoteq(__A, c) && chnoteq(__B, c);
}

#ifdef IO_HAS_CONNCEPTS
template<typename __CharType, __CharType __A, __CharType __B, __CharType __C>
	requires( is_charater_v<__CharType> )
#else
template<
	typename __CharType, __CharType __A, __CharType __B, __CharType __C,
	typename std::enable_if<
		is_charater<__CharType>::value
	>::type* = nullptr
>
#endif // IO_HAS_CONNCEPTS
constexpr bool is_one_of(__CharType c) noexcept
{
	return cheq(__A, c) || cheq(__B, c) || cheq(__C, c);
}

#ifdef IO_HAS_CONNCEPTS
template<typename __CharType, __CharType __A, __CharType __B, __CharType __C>
	requires( is_charater_v<__CharType> )
#else
template<
	typename __CharType, __CharType __A, __CharType __B, __CharType __C,
	typename std::enable_if<
		is_charater<__CharType>::value
	>::type* = nullptr
>
#endif // IO_HAS_CONNCEPTS
constexpr bool is_none_of(__CharType c) noexcept
{
	return chnoteq(__A, c) && chnoteq(__B, c) && chnoteq(__C, c);
}

#ifdef IO_HAS_CONNCEPTS
template<typename __CharType, __CharType __A, __CharType __B, __CharType __C, __CharType __D>
	requires( is_charater_v<__CharType> )
#else
template<
	typename __CharType, __CharType __A, __CharType __B, __CharType __C, __CharType __D,
	typename std::enable_if<
		is_charater<__CharType>::value
	>::type* = nullptr
>
#endif // IO_HAS_CONNCEPTS
constexpr bool is_one_of(__CharType c) noexcept
{
	return cheq(__A, c) || cheq(__B, c) || cheq(__C, c) || cheq(__D, c);
}

#ifdef IO_HAS_CONNCEPTS
template<typename __CharType, __CharType __A, __CharType __B, __CharType __C, __CharType __D>
	requires( is_charater_v<__CharType> )
#else
template<
	typename __CharType, __CharType __A, __CharType __B, __CharType __C, __CharType __D,
	typename std::enable_if<
		is_charater<__CharType>::value
	>::type* = nullptr
>
#endif // IO_HAS_CONNCEPTS
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
static constexpr const unsigned int LAT_A = detail::unsign(U'a');
static constexpr const unsigned int LAT_A_CAP = detail::unsign(U'A');
static constexpr const unsigned int ZERRO = detail::unsign(U'0');
static constexpr const unsigned int SPACE = detail::unsign(U' ');
static constexpr const unsigned int TAB = detail::unsign(U'\t');
static constexpr const unsigned int WHITE_SPACES_COUNT = 5;

static constexpr const unsigned int LOWER_MASK = 32;
static constexpr const unsigned int UPPER_MASK = 0x5F;

#ifdef IO_HAS_CONNCEPTS
template<typename char_t>
	requires( is_charater_v<char_t> )
#else
template<
	typename char_t,
	typename std::enable_if<
		is_charater<char_t>::value
	>::type* = nullptr
>
#endif // IO_HAS_CONNCEPTS
constexpr bool is_alpha(const char_t ch) noexcept
{
	return ( (detail::unsign(ch) | BITS ) - LAT_A) < ALBT_SIZE;
}

#ifdef IO_HAS_CONNCEPTS
template<typename char_t>
	requires( is_charater_v<char_t> )
#else
template<
	typename char_t,
	typename std::enable_if<
		is_charater<char_t>::value
	>::type* = nullptr
>
#endif // IO_HAS_CONNCEPTS
constexpr bool is_digit(const char_t ch) noexcept
{
	return (detail::unsign(ch)-ZERRO) < 10;
}

/// Checks given character is an alphanumeric character
/// \param ch a character
/// \return whether ch is an alphanumeric character
#ifdef IO_HAS_CONNCEPTS
template<typename char_t>
	requires( is_charater_v<char_t> )
#else
template<
	typename char_t,
	typename std::enable_if<
		is_charater<char_t>::value
	>::type* = nullptr
>
#endif // IO_HAS_CONNCEPTS
constexpr bool is_alnum(const char_t ch) noexcept
{
	return is_alpha(ch) || is_digit(ch);
}

/// Checks given character is space character [\t\n\v\f\r ]
/// \param ch a character
/// \return whether ch is a space character
#ifdef IO_HAS_CONNCEPTS
template<typename char_t>
	requires( is_charater_v<char_t> )
#else
template<
	typename char_t,
	typename std::enable_if<
		is_charater<char_t>::value
	>::type* = nullptr
>
#endif // IO_HAS_CONNCEPTS
constexpr bool is_space(const char_t ch) noexcept
{
	return SPACE == detail::unsign(ch) || (detail::unsign(ch) - TAB) < WHITE_SPACES_COUNT;
}

/// Checks given character is UNICODE-latin1 upper case [A-Z] space character
/// \param ch a character
/// \return whether ch is a latin1 upper case
#ifdef IO_HAS_CONNCEPTS
template<typename char_t>
	requires( is_charater_v<char_t> )
#else
template<
	typename char_t,
	typename std::enable_if<
		is_charater<char_t>::value
	>::type* = nullptr
>
#endif // IO_HAS_CONNCEPTS
static constexpr bool is_uppercase_latin1(const char_t ch) noexcept
{
	return  (detail::unsign(ch)-LAT_A_CAP) < ALBT_SIZE;
}

/// Checks given character is from UNICODE-latin1 lower case [a-z] space
/// \param ch a character
/// \return whether ch is a latin1 lower case
#ifdef IO_HAS_CONNCEPTS
template<typename char_t>
	requires( is_charater_v<char_t> )
#else
template<
	typename char_t,
	typename std::enable_if<
		is_charater<char_t>::value
	>::type* = nullptr
>
#endif // IO_HAS_CONNCEPTS
static constexpr bool is_lowercase_latin1(const char_t ch) noexcept
{
	return (detail::unsign(ch)-LAT_A) < ALBT_SIZE;
}

/// Checks given character is from UNICODE-latin1  [A-Z,a-z] space character
/// \param ch a character
/// \return whether ch is a UNICODE-latin1 character
#ifdef IO_HAS_CONNCEPTS
template<typename char_t>
	requires( is_charater_v<char_t> )
#else
template<
	typename char_t,
	typename std::enable_if<
		is_charater<char_t>::value
	>::type* = nullptr
>
#endif // IO_HAS_CONNCEPTS
constexpr bool is_latin1(const char_t ch) noexcept
{
	return is_lowercase_latin1( ch ) || is_uppercase_latin1( ch );
}

/// Converts the given character to lower case if it is UNICODE-latin1
/// \param ch a character
/// \return a lower case result or same character if ch is not a latin1 upper case character
#ifdef IO_HAS_CONNCEPTS
template<typename char_t>
	requires( is_charater_v<char_t> )
#else
template<
	typename char_t,
	typename std::enable_if<
		is_charater<char_t>::value
	>::type* = nullptr
>
#endif // IO_HAS_CONNCEPTS
constexpr char_t latin1_to_lower(const char_t ch) noexcept
{
	return is_uppercase_latin1( ch ) ? static_cast<char_t>( detail::unsign(ch) | LOWER_MASK ) : ch;
}

/// Converts the given character to upper case if it is UNICODE-latin1
/// \param ch a character
/// \return a lower case result or same character if ch is not a latin1 lower case character
#ifdef IO_HAS_CONNCEPTS
template<typename char_t>
	requires( is_charater_v<char_t> )
#else
template<
	typename char_t,
	typename std::enable_if<
		is_charater<char_t>::value
	>::type* = nullptr
>
#endif // IO_HAS_CONNCEPTS
constexpr char_t latin1_to_upper(const char_t ch) noexcept
{
	return is_lowercase_latin1( ch ) ? char_t( static_cast<const char>(ch) & UPPER_MASK) : ch;
}

/// Checks given character is string ending character i.e. '\0'
/// \param ch a character
/// \return whether ch equaling to '\0'
#ifdef IO_HAS_CONNCEPTS
template<typename char_t>
	requires( is_charater_v<char_t> )
#else
template<
	typename char_t,
	typename std::enable_if<
		is_charater<char_t>::value
	>::type* = nullptr
>
#endif // IO_HAS_CONNCEPTS
constexpr bool is_endl(const char_t ch) noexcept
{
	return cheq(0,ch);
}

/// Checks given character is string not ending no character i.e. '\0'
/// \param ch a character
/// \return whether ch not equaling to '\0'
#ifdef IO_HAS_CONNCEPTS
template<typename char_t>
	requires( is_charater_v<char_t> )
#else
template<
	typename char_t,
	typename std::enable_if<
		is_charater<char_t>::value
	>::type* = nullptr
>
#endif // IO_HAS_CONNCEPTS
constexpr bool not_endl(const char_t ch) noexcept
{
	return chnoteq(0, ch);
}

/// Converts in place given 0-ro ending character array string UNICODE-latin1 symbols to upper case
/// \param str a string to modify
#ifdef IO_HAS_CONNCEPTS
template<typename char_t>
	requires( is_charater_v<char_t> )
#else
template<
	typename char_t,
	typename std::enable_if<
		is_charater<char_t>::value
	>::type* = nullptr
>
#endif // IO_HAS_CONNCEPTS
inline void upcase_latin1(char_t* str) noexcept
{
	while( not_endl( *str ) ) {
		*str = latin1_to_upper(*str);
		++str;
	}
}

/// Converts in place given 0-ro ending character array string UNICODE-latin1 symbols to lower case
/// \param str a string to modify
#ifdef IO_HAS_CONNCEPTS
template<typename char_t>
	requires( is_charater_v<char_t> )
#else
template<
	typename char_t,
	typename std::enable_if<
		is_charater<char_t>::value
	>::type* = nullptr
>
#endif // IO_HAS_CONNCEPTS
inline void downcase_latin1(char_t* str) noexcept
{
	while( not_endl( *str ) ) {
		*str = latin1_to_lower(*str);
		++str;
	}
}

/// Checks given character is string ending character i.e. '\0'
/// \param ch a character
/// \return whether ch equaling to '\0'
#ifdef IO_HAS_CONNCEPTS
template<typename char_t>
	requires( is_charater_v<char_t> )
#else
template<
	typename char_t,
	typename std::enable_if<
		is_charater<char_t>::value
	>::type* = nullptr
>
#endif // IO_HAS_CONNCEPTS
inline const char_t* strchrn(const char_t* s,const char_t c,const std::size_t max_len) noexcept
{
	return const_cast<const char_t*>( std::char_traits<char_t>::find(s, max_len, c) );
}

#define IO_SPACES "\t\n\v\f\r "
#define IO_WSPACES L"\t\n\v\f\r "

inline const char* skip_spaces(const char *str) noexcept
{
	return str + io_strspn(str, IO_SPACES);
}

inline const wchar_t* skip_spaces(const wchar_t *str) noexcept
{
	return str + std::wcsspn(str, IO_WSPACES);
}

inline char* skip_spaces(char *str) noexcept
{
	return str + io_strspn(str, IO_SPACES);
}

inline const unsigned char* skip_spaces(const unsigned char *str) noexcept
{
	return reinterpret_cast<const unsigned char*>( skip_spaces( reinterpret_cast<const char*>(str) ) );
}

inline const char* skip_spaces_n(const char *str, std::size_t n) noexcept
{
	std::size_t offset = io_strspn(str, IO_SPACES);
	return (offset < n) ? (str+offset) : str;
}

inline const wchar_t* skip_spaces_n(const wchar_t* str, std::size_t n) noexcept
{
	std::size_t offset = std::wcsspn(str, IO_WSPACES);
	return (offset < n) ? (str+offset) : str;
}

inline const char* skip_spaces_ranged(const char *str, const char *end) noexcept
{
	return skip_spaces_n(str, static_cast<std::size_t>(end-str) );
}


#ifdef IO_HAS_CONNCEPTS
template<typename char_t>
	requires( is_charater_v<char_t> )
#else
template<
	typename char_t,
	typename std::enable_if<
		is_charater<char_t>::value
	>::type* = nullptr
>
#endif // IO_HAS_CONNCEPTS
inline bool start_with(const char_t* s,const char_t* pattern,const std::size_t size) noexcept
{
	return 0 == std::char_traits<char_t>::compare( s, pattern, size );
}

#ifdef IO_HAS_CONNCEPTS
template<typename char_t>
	requires( is_charater_v<char_t> )
#else
template<
	typename char_t,
	typename std::enable_if<
		is_charater<char_t>::value
	>::type* = nullptr
>
#endif // IO_HAS_CONNCEPTS
inline bool start_with(const char_t* s,const char_t* pattern) noexcept
{
	return start_with( s, pattern, std::char_traits<char_t>::length(pattern) );
}

inline std::size_t str_size(const char* b, const char* e) noexcept
{
	return memory_traits::distance(b, e);
}

} // namespace io

#endif // __IO_STRING_ALGS_HPP_INCLUDED__
