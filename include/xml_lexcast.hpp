 /*
 * Copyright (c) 2016
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef __IO_XML_LEXCAST_HPP_INCLUDED__
#define __IO_XML_LEXCAST_HPP_INCLUDED__

#include "config.hpp"

#include <limits>
#include <cstdlib>
#include <ctime>

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

namespace io {

namespace {

template<typename int_t> struct int_limits {};

template<>
struct int_limits<uint8_t> {
	static constexpr const uint8_t min = 0;
	static constexpr const uint8_t max = UCHAR_MAX;
	static constexpr const uint8_t max_str_len = 4;
};

template<>
struct int_limits<int8_t> {
	static constexpr const int8_t min = SCHAR_MIN;
	static constexpr const int8_t max = SCHAR_MAX;
	static constexpr const uint8_t max_str_len = 5;
};

template<>
struct int_limits<uint16_t> {
	static constexpr const uint16_t min = 0;
	static constexpr const uint16_t max = USHRT_MAX;
	static constexpr const uint8_t max_str_len = 6;
};

template<>
struct int_limits<int16_t> {
	static constexpr const uint16_t min = SHRT_MIN;
	static constexpr const uint16_t max = SHRT_MAX;
	static constexpr const uint8_t max_str_len = 7;
};

template<>
struct int_limits<uint32_t> {
	static constexpr const uint32_t min = 0;
	static constexpr const uint32_t max = UINT_MAX;
	static constexpr const uint8_t  max_str_len = 11;
};


template<>
struct int_limits<int32_t> {
	static constexpr const uint32_t min = INT_MIN;
	static constexpr const uint32_t max = INT_MAX;
	static constexpr const uint8_t  max_str_len = 12;
};

template<>
struct int_limits<uint64_t> {
	static constexpr const uint64_t min = 0;
	static constexpr const uint64_t max = ULLONG_MAX;
	static constexpr const uint8_t max_str_len = 21;
};

template<>
struct int_limits<int64_t> {
#if defined(LLONG_MIN) && defined(LLONG_MAX)
	static constexpr const int64_t min = LLONG_MIN;
	static constexpr const int64_t max = LLONG_MAX;
#elif defined(_I64_MIN) && defined(_I64_MAX)
	static constexpr const int64_t min = LLONG_MIN;
	static constexpr const int64_t max = LLONG_MAX;
#else
	static constexpr const int64_t min = -9223372036854775808LL;
	static constexpr const int64_t max = 9223372036854775807LL;
#endif // defined
	static constexpr const uint8_t  max_str_len = 22;
};

template<typename _char_t>
struct strings {};

template<>
struct strings<char> {
	typedef char char_type;
	static constexpr const char  zerro_char = '0';
	static constexpr const char  minus_char = '-';
	static constexpr const char* zerro = "0";
	static constexpr const char* max_uint_8 = "255";
	static constexpr const char* min_int_8 = "–128";
	static constexpr const char* max_int_8 = "127";
	static constexpr const char* max_uint_16 = "65535";
	static constexpr const char* max_uint_32 = "4294967295";
	static constexpr const char* max_uint_64 = "18446744073709551615";
	static constexpr const char* min_int16 = "–32768";
	static constexpr const char* max_int16 = "32767";
	static constexpr const char* min_int32 = "–2147483648";
	static constexpr const char* max_int32 = "2147483647";
	static constexpr const char* min_int64 = "-9223372036854775808";
	static constexpr const char* max_int64 = "9223372036854775807";
	static constexpr const char* true_str = "true";
	static constexpr const char* false_str = "false";
};

template<>
struct strings<wchar_t> {
	typedef wchar_t char_type;
	static constexpr const wchar_t  zerro_char = L'0';
	static constexpr const wchar_t  minus_char = '-';
	static constexpr const wchar_t* zerro = L"0";
	static constexpr const wchar_t* max_int_8 = L"127";
	static constexpr const wchar_t* max_uint_16 = L"65535";
	static constexpr const wchar_t* max_uint_32 = L"4294967295";
	static constexpr const wchar_t* max_uint_64 = L"18446744073709551615";
	static constexpr const wchar_t* min_int_8 = L"–128";
	static constexpr const wchar_t* max_uint_8 = L"255";
	static constexpr const wchar_t* min_int16 = L"–32768";
	static constexpr const wchar_t* max_int16 = L"32767";
	static constexpr const wchar_t* min_int32 = L"–2147483648";
	static constexpr const wchar_t* max_int32 = L"2147483647";
	static constexpr const wchar_t* min_int64 = L"-9223372036854775808";
	static constexpr const wchar_t* max_int64 = L"9223372036854775807";
	static constexpr const wchar_t* true_str = L"true";
	static constexpr const wchar_t* false_str = L"false";
};

template<>
struct strings<char16_t> {
	typedef char16_t char_type;
	static constexpr const char16_t  zerro_char = u'0';
	static constexpr const char16_t  minus_char = u'-';
	static constexpr const char16_t* zerro = u"0";
	static constexpr const char16_t* max_uint_8 = u"255";
	static constexpr const char16_t* max_uint_16 = u"65535";
	static constexpr const char16_t* max_uint_32 = u"4294967295";
	static constexpr const char16_t* max_uint_64 = u"18446744073709551615";
	static constexpr const char16_t* min_int_8 = u"–128";
	static constexpr const char16_t* max_int_8 = u"127";
	static constexpr const char16_t* min_int16 = u"–32768";
	static constexpr const char16_t* max_int16 = u"32767";
	static constexpr const char16_t* min_int32 = u"–2147483648";
	static constexpr const char16_t* max_int32 = u"2147483647";
	static constexpr const char16_t* min_int64 = u"-9223372036854775808";
	static constexpr const char16_t* max_int64 = u"9223372036854775807";
	static constexpr const char16_t* true_str = u"true";
	static constexpr const char16_t* false_str = u"false";
};

template<>
struct strings<char32_t> {
	typedef char32_t char_type;
	static constexpr const char32_t  zerro_char = U'0';
	static constexpr const char32_t  minus_char = U'-';
	static constexpr const char32_t* zerro = U"0";
	static constexpr const char32_t* max_int_8 = U"127";
	static constexpr const char32_t* max_uint_16 = U"65535";
	static constexpr const char32_t* max_uint_32 = U"4294967295";
	static constexpr const char32_t* max_uint_64 = U"18446744073709551615";
	static constexpr const char32_t* min_int_8 = U"–128";
	static constexpr const char32_t* max_uint_8 = U"255";
	static constexpr const char32_t* min_int16 = U"–32768";
	static constexpr const char32_t* max_int16 = U"32767";
	static constexpr const char32_t* min_int32 = U"–2147483648";
	static constexpr const char32_t* max_int32 = U"2147483647";
	static constexpr const char32_t* min_int64 = U"-9223372036854775808";
	static constexpr const char32_t* max_int64 = U"9223372036854775807";
	static constexpr const char32_t* true_str = U"true";
	static constexpr const char32_t* false_str = U"false";
};



template<typename __int_type, typename __char_type>
struct cast_traits {
	typedef __int_type int_type;
	typedef __char_type char_type;
	typedef strings<char_type> strings_type;
private:
	typedef int_limits<__int_type> limits_type;
	template<typename I,class _mxt> struct strs {};

	template<class _mxt> struct strs<uint8_t,_mxt> {
		typedef typename _mxt::char_type chr;
		static constexpr const chr* min_str = strings_type::zerro;
		static constexpr const chr* max_str = strings_type::max_uint_8;
	};

	template<class _mxt> struct strs<int8_t,_mxt> {
		typedef typename _mxt::char_type chr;
		static constexpr const chr* min_str = strings_type::min_int_8;
		static constexpr const chr* max_str = strings_type::max_int_8;
	};

	template<class _mxt> struct strs<uint16_t,_mxt> {
		typedef typename _mxt::char_type chr;
		static constexpr const chr* min_str = strings_type::zerro;
		static constexpr const chr* max_str = strings_type::max_uint_16;
	};


	template<class _mxt> struct strs<int16_t,_mxt> {
		typedef typename _mxt::char_type chr;
		static constexpr const chr* min_str = strings_type::min_int16;
		static constexpr const chr* max_str = strings_type::max_int16;
	};


	template<class _mxt> struct strs<uint32_t,_mxt> {
		typedef typename _mxt::char_type chr;
		static constexpr const chr* min_str = strings_type::zerro;
		static constexpr const chr* max_str = strings_type::max_uint_32;
	};

	template<class _mxt> struct strs<int32_t,_mxt> {
		typedef typename _mxt::char_type chr;
		static constexpr const chr* min_str = strings_type::min_int32;
		static constexpr const chr* max_str = strings_type::max_int32;
	};

	template<class _mxt> struct strs<uint64_t,_mxt> {
		typedef typename _mxt::char_type chr;
		static constexpr const chr* min_str = strings_type::zerro;
		static constexpr const chr* max_str = strings_type::max_uint_64;
	};

	template<class _mxt> struct strs<int64_t,_mxt> {
		typedef typename _mxt::char_type chr;
		static constexpr const chr* min_str = strings_type::min_int64;
		static constexpr const chr* max_str = strings_type::max_int64;
	};
public:
	static constexpr const char_type zerro_char = strings_type::zerro_char;
	static constexpr const char_type minus_char = strings_type::minus_char;
	static constexpr const int_type min = limits_type::min;
	static constexpr const int_type max = limits_type::max;
	static constexpr const uint8_t max_str_len = limits_type::max_str_len;
	static constexpr const char_type* zerro_str = strings_type::zerro;
	static constexpr const char_type* min_str = strs<int_type,strings_type>::min_str;
	static constexpr const char_type* max_str = strs<int_type,strings_type>::max_str;
};

// Fast codecvt convert for ascii/latin1 (UTF-8 0,127) any with UCS
// simply cast to short/int versions
// for unicode 0-9, A-Z, a-z it is always ok
template<typename __char_type>
struct move_digits {
private:
	typedef std::char_traits<char> ch_8_tr;
	typedef std::char_traits<__char_type> ch_n_tr;
public:
	static inline char* move(char* dst, const __char_type* src)
	{
		std::size_t len = ch_n_tr::length(src)+1;
		char *d = dst + len;
		const __char_type *s = src + len;
		do {
			*d = ch_8_tr::to_char_type( ch_n_tr::to_int_type( *s ) );
			--s; --d;
		} while( d >= dst);
		return dst + len;
	}
	static inline __char_type* move(__char_type* dst, const char* src)
	{
		std::size_t len = ch_8_tr::length(src)+1;
		__char_type *d = dst + len;
		const char *s = src + len;
		do {
			*d = ch_n_tr::to_char_type( ch_8_tr::to_int_type( *s ) );
			--s;--d;
		} while( d >= dst);
		return dst + len;
	}

};

// when source and destinations string is a byte/multibyte
template<> struct move_digits<char> {
	static inline char* move(char* dst, const char* src)
	{
		return std::char_traits<char>::move(dst, src, std::char_traits<char>::length(src) );
	}
};

template<typename __int_type, typename __char_t>
struct generic_uint_cast {
	typedef __int_type int_type;
	typedef __char_t char_type;
	typedef std::char_traits<char_type> char_traits;
	typedef cast_traits<int_type,char_type> cast_traits;
private:
	static constexpr const __int_type RADIX10 = static_cast<int_type>(10);
	// Replace this if CPU not supporting native div/mod
	// and/or complier shows bad optimization results
	static inline uint_fast8_t divmod(int_type& n,uint_fast8_t radix) noexcept
	{
		uint_fast8_t rem = uint_fast8_t(n % radix);
		n /= static_cast<int_type>(radix);
		return rem;
	}
	static inline char_type* format_u(int_type value,char_type* to) noexcept IO_NO_INLINE
	{
		char_type result[cast_traits::max_str_len];
		char_type *s = &result[cast_traits::max_str_len];
		*s = char_traits::to_char_type(0);
		std::size_t len = 1;
		do {
			*(--s) = cast_traits::zerro_char + divmod(value, RADIX10);
			++len;
		} while( 0 != value );
		return char_traits::move( to, s, len );
	}
public:

	static const char_type* utoa(int_type value,char_type* to) noexcept
	{
		switch(value) {
		case 0:
			char_traits::move(to,cast_traits::min_str,2);
			return to;
		case cast_traits::max:
			char_traits::move(to, cast_traits::max_str, cast_traits::max_str_len);
			return to;
		default:
			break;
		}
		return format_u(value, to);
	}

	static uint32_t str_to_dword(const char_type* str) noexcept
	{
		char* s = static_cast<char*>( io_alloca( char_traits::length(str) ) );
		move_digits<char_type>::move(s,str);
		return static_cast<uint32_t>( std::strtoul(s, nullptr, 0) );
	}

	static uint64_t str_to_qword(const char_type* str) noexcept
	{
		char* s = static_cast<char*>( io_alloca( char_traits::length(str) ) );
		move_digits<char_type>::move(s,str);
		return static_cast<uint64_t>( std::strtoull(s, nullptr, 0) );
	}
};

template<typename __int_type, typename __char_t>
struct generic_int_cast {
	typedef __int_type int_type;
	typedef __char_t char_type;
	typedef std::char_traits<char_type> char_traits;
	typedef cast_traits<int_type,char_type> cast_traits;
private:
	static constexpr const __int_type RADIX10 = static_cast<int_type>(10);
	static inline int8_t my_abs(int8_t x)
	{
	#ifdef io_abs
		return io_abs(x);
	#else
		int8_t minus_flag = x >> 0x1F;
		return (minus_flag ^ x) - minus_flag;
	#endif
	}
	// Replace this if CPU not supporting native div/mod
	// and/or complier shows bad optimization results
	// optimize with -O3 shows best results
	static inline uint_fast8_t divmod(int_type& n,uint_fast8_t radix) noexcept
	{
		uint_fast8_t rem = my_abs( uint_fast8_t(n % radix) );
		n /= static_cast<int_type>(radix);
		return rem;
	}
	static inline const char_type* format_a(int_type value,char_type* to) noexcept IO_NO_INLINE
	{
		char_type result[cast_traits::max_str_len];
		char_type *s = &result[cast_traits::max_str_len];
		bool need_msgn = value < 0;
		*s = char_traits::to_char_type(0);
		std::size_t len = need_msgn ? 2 : 1;
		do {
			*(--s) =  cast_traits::zerro_char +  divmod(value,RADIX10);
			++len;
		} while(0 != value);
		if(need_msgn) *(--s) = cast_traits::minus_char;
		return char_traits::move( to, s, len );
	}
public:

	static const char_type* itoa(int_type value,char_type* to) noexcept
	{
		switch(value) {
		case 0:
			char_traits::move(to, cast_traits::zerro_str, 2);
			return to;
		case cast_traits::min:
			char_traits::move(to,cast_traits::min_str,cast_traits::max_str_len);
			return to;
		case cast_traits::max:
			char_traits::move(to, cast_traits::max_str, cast_traits::max_str_len);
			return to;
		default:
			break;
		}
		return format_a(value, to);
	}

	static int32_t str_to_int(const char_type* str) noexcept
	{
		char* s = static_cast<char*>( io_alloca( char_traits::length(str) ) );
		move_digits<char_type>::move(s,str);
		return static_cast<int32_t>( std::strtol(s, nullptr, 0) );
	}

	static int64_t str_to_long(const char_type* str) noexcept
	{
		char* s = static_cast<char*>( io_alloca( char_traits::length(str) ) );
		move_digits<char_type>::move(s,str);
		return static_cast<int64_t>( std::strtoll(s, nullptr, 0) );
	}

};

}  // namespace

namespace xml {

template<typename __char_t>
struct lexical_cast {
public:
	typedef __char_t char_type;
	typedef std::char_traits<char_type> char_traits;
private:
	typedef strings<char_type> strings;
public:

	static inline const char_type* byte_to_str(uint8_t v,char_type* to) noexcept
	{
		return generic_uint_cast<uint8_t,char_type>::utoa(v,to);
	}

	static inline uint8_t str_to_byte(const char_type* str) noexcept
	{
		return static_cast<uint8_t>( generic_uint_cast<uint32_t,char_type>::str_to_dword(str) );
	}

	static inline const char_type* small_to_str(int8_t v,char_type* to) noexcept
	{
		return generic_int_cast<int8_t,char_type>::itoa(v,to);
	}

	static inline int8_t str_to_small(const char_type* str) noexcept
	{
		return static_cast<int8_t>( generic_int_cast<uint32_t,char_type>::str_to_int(str) );
	}

	static inline const char_type* word_to_str(uint16_t v,char_type* to) noexcept
	{
		return generic_uint_cast<uint16_t,char_type>::utoa(v,to);
	}

	static inline uint16_t str_to_word(const char_type* str) noexcept
	{
		return static_cast<uint16_t>( generic_uint_cast<uint32_t,char_type>::str_to_dword(str) );
	}

	static inline const char_type* short_to_str(int16_t v,char_type* to) noexcept
	{
		return generic_int_cast<int16_t,char_type>::itoa(v,to);
	}

	static inline int16_t str_to_short(const char_type* str) noexcept
	{
		return static_cast<int16_t>( generic_int_cast<uint32_t,char_type>::str_to_int(str) );
	}

	static inline const char_type* dword_to_str(uint32_t v,char_type* to) noexcept
	{
		return generic_uint_cast<uint32_t,char_type>::utoa(v,to);
	}

	static inline uint32_t str_to_dword(const char_type* str) noexcept
	{
		return generic_uint_cast<uint32_t,char_type>::str_to_dword(str);
	}

	static inline const char_type* int_to_str(int32_t v,char_type* to) noexcept
	{
		return generic_int_cast<int32_t,char_type>::itoa(v,to);
	}

	static inline int32_t str_to_int(const char_type* str) noexcept
	{
		return generic_int_cast<uint32_t,char_type>::str_to_int(str);
	}

	static inline const char_type* qword_to_str(uint64_t v,char_type* to) noexcept
	{
		return generic_uint_cast<uint64_t,char_type>::utoa(v,to);
	}

	static inline uint64_t str_to_qword(const char_type* str) noexcept
	{
		return generic_uint_cast<uint64_t,char_type>::str_to_qword(str);
	}

	static inline const char_type* long_to_str(int64_t v,char_type* to) noexcept
	{
		return generic_int_cast<int64_t,char_type>::itoa(v,to);
	}

	static inline int64_t str_to_long(const char_type* str) noexcept
	{
		return generic_int_cast<int64_t,char_type>::str_to_long(str);
	}

	static inline const char_type* float_to_str(float v, char_type* to) noexcept {
		char buff[32];
		std::sprintf(buff, "%G", buff);
		return move_digits<char_type>::move( to, buff);
	}

	static inline float str_to_float(const char_type* str) noexcept {
		char buff[32];
		move_digits<char_type>::move( buff, str);
		return std::strtof(buff, nullptr);
	}

	static inline const char_type* double_to_str(double v, char_type* to) noexcept {
		char buff[64];
		std::sprintf(buff, "%G", buff);
		return move_digits<char_type>::move( to, buff);
	}

	static inline double str_to_double(const char_type* str) noexcept {
		char buff[64];
		move_digits<char_type>::move( buff, str);
		return std::strtod(buff, nullptr);
	}

	static inline const char_type* longdouble_to_str(long double v,char_type* to) noexcept {
		char buff[128];
		std::sprintf(buff, "%G", buff);
		return move_digits<char_type>::move( to, buff);
	}

	static inline long double str_to_longdouble(const char_type* str) noexcept {
		char buff[128];
		move_digits<char_type>::move( buff, str);
		return std::strtold(buff, nullptr);
	}

	static inline const char_type* boolean_to_str(bool value, char_type* to) noexcept {
		if(value) {
			char_traits::move(to, strings::true_str, 5);
		} else {
			char_traits::move(to, strings::false_str, 6);
		}
		return to;
	}

	static inline bool str_to_boolean(const char_type* from) noexcept {
		return 0 == char_traits::compare(from, strings::true_str, 5);
	}

};

typedef lexical_cast<char> char8_lexical_cast;
typedef lexical_cast<uint16_t> char16_lexical_cast;
typedef lexical_cast<uint32_t> char32_lexical_cast;
typedef lexical_cast<wchar_t> wchar_lexical_cast;

template<typename T, class __lex_cast = char8_lexical_cast>
class lexical_cast_traits
{
};

template<class __lex_cast>
class lexical_cast_traits<uint8_t,__lex_cast>
{
private:
	typedef __lex_cast lext_cast;
public:
	typedef typename lext_cast::char_type char_type;
	static constexpr uint8_t max_str_len = int_limits<uint8_t>::max_str_len;
	static inline void  to_stiring(uint8_t byte, const char_type* to) noexcept {
		return lext_cast::byte_to_str(byte, to);
	}
	static inline uint8_t from_stiring(const char_type* from) noexcept {
		return lext_cast::str_to_byte(from);
	}
};


template<class __lex_cast>
class lexical_cast_traits<int8_t,__lex_cast>
{
private:
	typedef __lex_cast lext_cast;
public:
	typedef typename lext_cast::char_type char_type;
	static constexpr uint8_t max_str_len = int_limits<int8_t>::max_str_len;
	static inline void to_string(int8_t small, const char_type* to) noexcept {
		return lext_cast::small_to_str(small, to);
	}
	static inline int8_t from_string(const char_type* from) noexcept {
		return lext_cast::str_to_small(from);
	}
};

template<class __lex_cast>
class lexical_cast_traits<uint16_t,__lex_cast>
{
private:
	typedef __lex_cast lext_cast;
public:
	typedef typename lext_cast::char_type char_type;
	static constexpr uint8_t max_str_len = int_limits<int8_t>::max_str_len;
	static inline void to_string(uint16_t word, const char_type* to) noexcept {
		return lext_cast::word_to_str(word, to);
	}
	static inline uint16_t from_string(const char_type* from) noexcept {
		return lext_cast::str_to_word(from);
	}
};

template<class __lex_cast>
class lexical_cast_traits<int16_t,__lex_cast>
{
private:
	typedef __lex_cast lext_cast;
public:
	typedef typename lext_cast::char_type char_type;
	static constexpr uint8_t max_str_len = int_limits<int16_t>::max_str_len;
	static inline void to_string(int16_t small, const char_type* to) noexcept {
		return lext_cast::short_to_str(small, to);
	}
	static inline uint16_t from_string(const char_type* from) noexcept {
		return lext_cast::str_to_short(from);
	}
};


template<class __lex_cast>
class lexical_cast_traits<uint32_t,__lex_cast>
{
private:
	typedef __lex_cast lext_cast;
public:
	typedef typename lext_cast::char_type char_type;
	static constexpr uint8_t max_str_len = int_limits<uint32_t>::max_str_len;
	static inline void to_string(int32_t dword, const char_type* to) noexcept {
		return lext_cast::dword_to_str(dword, to);
	}
	static inline uint32_t from_string(const char_type* from) noexcept {
		return lext_cast::str_to_dword(from);
	}
};

template<class __lex_cast>
class lexical_cast_traits<int32_t,__lex_cast>
{
private:
	typedef __lex_cast lext_cast;
public:
	typedef typename lext_cast::char_type char_type;
	static constexpr uint8_t max_str_len = int_limits<int32_t>::max_str_len;
	static inline void to_string(int32_t dword, const char_type* to) noexcept {
		return lext_cast::int_to_str(dword, to);
	}
	static inline uint32_t from_string(const char_type* from) noexcept {
		return lext_cast::str_to_int(from);
	}
};

template<class __lex_cast>
class lexical_cast_traits<uint64_t,__lex_cast>
{
private:
	typedef __lex_cast lext_cast;
public:
	typedef typename lext_cast::char_type char_type;
	static constexpr uint8_t max_str_len = int_limits<uint64_t>::max_str_len;
	static inline void to_string(uint64_t qword, const char_type* to) noexcept {
		return lext_cast::qword_to_str(qword, to);
	}
	static inline uint64_t from_string(const char_type* from) noexcept {
		return lext_cast::str_to_qword(from);
	}
};

template<class __lex_cast>
class lexical_cast_traits<int64_t,__lex_cast>
{
private:
	typedef __lex_cast lext_cast;
public:
	typedef typename lext_cast::char_type char_type;
	static constexpr uint8_t max_str_len = int_limits<int64_t>::max_str_len;
	static inline void to_string(int64_t llong, const char_type* to) noexcept {
		return lext_cast::long_to_str(llong, to);
	}
	static inline int64_t from_string(const char_type* from) noexcept {
		return lext_cast::str_to_long(from);
	}
};

template<class __lex_cast>
class lexical_cast_traits<float,__lex_cast>
{
private:
	typedef __lex_cast lext_cast;
public:
	typedef typename lext_cast::char_type char_type;
	static constexpr uint8_t max_str_len = 32;
	static inline void to_string(float real, const char_type* to) noexcept {
		return lext_cast::float_to_str(real, to);
	}
	static inline float from_string(const char_type* from) noexcept {
		return lext_cast::str_to_long(from);
	}
};

template<class __lex_cast>
class lexical_cast_traits<double,__lex_cast>
{
private:
	typedef __lex_cast lext_cast;
public:
	typedef typename lext_cast::char_type char_type;
	static constexpr uint8_t max_str_len = 64;
	static inline void to_string(double deximal, const char_type* to) noexcept {
		return lext_cast::double_to_str(deximal, to);
	}
	static inline double from_string(const char_type* from) noexcept {
		return lext_cast::str_to_dobule(from);
	}
};

template<class __lex_cast>
class lexical_cast_traits<long double,__lex_cast>
{
private:
	typedef __lex_cast lext_cast;
public:
	typedef typename lext_cast::char_type char_type;
	static constexpr uint8_t max_str_len = 128;
	static inline void to_string(long double big_deximal, const char_type* to) noexcept {
		return lext_cast::longdouble_to_str(big_deximal, to);
	}
	static inline long double from_string(const char_type* from) noexcept {
		return lext_cast::str_to_longdouble(from);
	}
};

template<class __lex_cast>
class lexical_cast_traits<bool,__lex_cast>
{
private:
	typedef __lex_cast lext_cast;
public:
	typedef typename lext_cast::char_type char_type;
	static constexpr uint8_t max_str_len = 6;
	static inline void to_string(bool __b, const char_type* to) noexcept {
		return lext_cast::boolean_to_str(__b, to);
	}
	static inline long double from_string(const char_type* from) noexcept {
		return lext_cast::str_to_boolean(from);
	}
};

} // namesapce xml

} // namespace io


#endif // __IO_XML_LEXCAST_HPP_INCLUDED__