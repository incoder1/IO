/*
 *
 * Copyright (c) 2016-2020
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

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE


#include <stdint.h>
#include <cstdlib>
#include <ctime>

#include <limits.h>
#include <limits>


namespace io {

namespace {

template<typename __integer_type__> struct int_limits {};

template<>
struct int_limits<uint8_t> {
	static constexpr const uint8_t min = 0;
#ifdef __GNUG__
	static constexpr const uint8_t max = __UINT8_MAX__;
#else
	static constexpr const uint8_t max = UCHAR_MAX;
#endif
	static constexpr const uint8_t max_str_len = std::numeric_limits<uint8_t>::digits + 1;
};

template<>
struct int_limits<int8_t> {
#ifdef __GNUG__
	static constexpr const int8_t min = static_cast<int8_t>(-__INT8_MAX__ - 1);
	static constexpr const int8_t max = __INT8_MAX__;
#else
	static constexpr const int8_t min = CHAR_MIN;
	static constexpr const int8_t max = CHAR_MAX;
#endif // __GNUG__
	static constexpr const uint8_t max_str_len = std::numeric_limits<uint8_t>::digits + 2;
};

template<>
struct int_limits<uint16_t> {
	static constexpr const uint16_t min = 0;
#ifdef __GNUG__
	static constexpr const uint16_t max = __UINT16_MAX__;
#else
	static constexpr const uint16_t max = USHRT_MAX;
#endif // __GNUG__
	static constexpr const uint8_t max_str_len = std::numeric_limits<uint16_t>::digits;
};

template<>
struct int_limits<int16_t> {
#ifdef __GNUG__
	static constexpr const uint16_t min = static_cast<uint16_t>(-__INT16_MAX__ - 1);
	static constexpr const uint16_t max = __INT16_MAX__;
#else
	static constexpr const uint16_t min = SHRT_MIN;
	static constexpr const uint16_t max = SHRT_MAX;
#endif // __GNUG__
	static constexpr const uint8_t max_str_len = 7;
};

template<>
struct int_limits<uint32_t> {
#ifdef __GNUG__
	static constexpr const uint32_t min = 0;
	static constexpr const uint32_t max = __UINT32_MAX__;
#else
	static constexpr const uint32_t min = 0;
	static constexpr const uint32_t max = UINT_MAX;
#endif // __GNUG__
	static constexpr const uint8_t  max_str_len = 11;
};


template<>
struct int_limits<int32_t> {
#ifdef __GNUG__
	static constexpr const uint32_t min = static_cast<uint32_t>(-__INT32_MAX__ - 1);
	static constexpr const uint32_t max = __INT32_MAX__;
#else
	static constexpr const uint32_t min = INT_MIN;
	static constexpr const uint32_t max = INT_MAX;
#endif // __GNUG__
	static constexpr const uint8_t  max_str_len = 12;
};

template<>
struct int_limits<uint64_t> {
	static constexpr const uint64_t min = 0ULL;
#ifdef __GNUG__
	static constexpr const uint64_t max = __UINT64_MAX__;
#else
	static constexpr const uint64_t max = ULLONG_MAX;
#endif // __GNUG__
	static constexpr const uint8_t max_str_len = std::numeric_limits<int64_t>::digits + 1;
};

template<>
struct int_limits<int64_t> {
#ifdef __GNUG__
	static constexpr const uint64_t min = static_cast<uint64_t>(-__INT64_MAX__ - 1);
	static constexpr const uint64_t max = __INT64_MAX__;
#else
	static constexpr const uint64_t min = LLONG_MIN;
	static constexpr const uint64_t max = LLONG_MAX;
#endif // defined
	static constexpr const uint8_t  max_str_len = std::numeric_limits<int64_t>::digits + 2;
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
	static constexpr const wchar_t  minus_char = L'-';
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
// for UNICODE 0-9, A-Z, a-z it is always ok
template<typename __char_type>
struct move_digits {
private:
	typedef std::char_traits<char> ch_8_tr;
	typedef std::char_traits<__char_type> ch_n_tr;
public:
	static inline char* move(char* dst, const __char_type* src) {
		std::size_t len = ch_n_tr::length(src)+1;
		char *d = dst + len;
		const __char_type *s = src + len;
		do {
			*d = ch_8_tr::to_char_type( ch_n_tr::to_int_type( *s ) );
			--s;
			--d;
		}
		while( d >= dst);
		return dst + len;
	}
	static inline __char_type* move(__char_type* dst, const char* src) {
		std::size_t len = ch_8_tr::length(src)+1;
		__char_type *d = dst + len;
		const char *s = src + len;
		do {
			*d = ch_n_tr::to_char_type( ch_8_tr::to_int_type( *s ) );
			--s;
			--d;
		}
		while( d >= dst);
		return dst + len;
	}

};

// when source and destinations string is a byte/multibyte
template<> struct move_digits<char> {
	static inline char* move(char* dst, const char* src) {
		return std::char_traits<char>::move(dst, src, std::char_traits<char>::length(src) );
	}
};

template<typename __int_type, typename __char_t>
struct generic_uint_cast {
	typedef __int_type int_type;
	typedef __char_t char_type;
	typedef std::char_traits<char_type> char_traits;
	typedef cast_traits<int_type,char_type> ctraits;
private:
	// Replace this if CPU not supporting native div/mod
	// and/or complier shows bad optimization results
	static inline uint8_t divmod(int_type& n) noexcept {
		uint8_t rem = n % 10;
		n /= 10;
		return rem;
	}
	static inline char_type* format_u(int_type value, char_type* to) noexcept
	{
		char_type result[ctraits::max_str_len];
		char_type *s = &result[ctraits::max_str_len];
		*s = char_traits::to_char_type(0);
		std::size_t len = 1;
		do {
			*(--s) = ctraits::zerro_char + divmod(value);
			++len;
		}
		while( 0 != value );
		return char_traits::move( to, s, len );
	}

public:

	static const char_type* utoa(int_type value,char_type* to) noexcept {
		switch(value) {
		case 0:
			char_traits::move(to,ctraits::min_str,2);
			return to;
		case ctraits::max:
			char_traits::move(to, ctraits::max_str, ctraits::max_str_len);
			return to;
		default:
			break;
		}
		return format_u(value, to);
	}

	static const int_type str_to_u(const char_type* str,const char_type** endp) noexcept {
		char* s = static_cast<char*>( io_alloca( char_traits::length(str) ) );
		move_digits<char_type>::move(s,str);
		char *ep;
		int_type ret = static_cast<int_type>( std::strtoull(s, &ep, 10) );
		if(nullptr != ep)
			*endp = nullptr;
		else
			*endp = const_cast<char_type*>(str + memory_traits::distance(s,ep));
		return ret;
	}

};

template<typename __int_type, typename __char_t>
struct generic_int_cast {
	typedef __int_type int_type;
	typedef __char_t char_type;
	typedef std::char_traits<char_type> char_traits;
	typedef cast_traits<int_type,char_type> ctraits;
private:
	static inline int8_t my_abs(int8_t x) {
#ifdef io_abs
		return io_abs(x);
#else
		int8_t minus_flag = x >> 7;
		return (minus_flag ^ x) - minus_flag;
#endif
	}
	// Replace this if CPU not supporting native div/mod
	// and/or complier shows bad optimization results
	// optimize with -O3 shows best results
	static inline uint8_t divmod(int_type& n) noexcept {
		uint8_t rem = n % 10;
		n /= 10;
		return rem;
	}
	static const char_type* format_a(int_type value,char_type* to) noexcept {
		char_type result[ctraits::max_str_len];
		char_type *s = &result[ctraits::max_str_len];
		bool need_msgn = value < 0;
		if(need_msgn)
			value = -value;
		*s = char_traits::to_char_type(0);
		std::size_t len = need_msgn ? 2 : 1;
		do {
			*(--s) =  ctraits::zerro_char +  divmod(value);
			++len;
		}
		while(0 != value);
		if(need_msgn)
			*(--s) = ctraits::minus_char;
		return char_traits::move( to, s, len );
	}

	static int_type strtoi(const char *s, char* const *endptr) noexcept {
		return static_cast<__int_type>( std::strtoll(s, endptr, 10) );
	}
public:

	static const char_type* itoa(int_type value,char_type* to) noexcept {
		switch(value) {
		case 0:
			char_traits::move(to, ctraits::zerro_str, 2);
			return to;
		case ctraits::min:
			char_traits::move(to,ctraits::min_str,ctraits::max_str_len);
			return to;
		case ctraits::max:
			char_traits::move(to, ctraits::max_str, ctraits::max_str_len);
			return to;
		default:
			return format_a(value, to);
		}
	}

	static int_type str_to_i(const char_type* str, const char_type** endp) noexcept {
		char* s = static_cast<char*>( io_alloca( char_traits::length(str) ) );
		move_digits<char_type>::move(s,str);
		char *ep;
		int_type ret = strtoi(s, &ep, 0);
		if(nullptr != ep)
			*endp = nullptr;
		else
			*endp = const_cast<char_type*>(str + memory_traits::distance(s,ep));
		return ret;
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
	typedef strings<char_type> strings_traits;

	template<typename T>
	static constexpr unsigned int float_max_digits() {
		typedef std::numeric_limits<T> limits_type;
		return 3 + limits_type::digits - limits_type::min_exponent;
	}

	static constexpr std::size_t FLOAT_MAX_DIGITS = float_max_digits<float>();
	static constexpr std::size_t DOUBLE_MAX_DIGITS = float_max_digits<double>();
	static constexpr std::size_t LONG_DOUBLE_MAX_DIGITS = float_max_digits<long double>();

public:

	static inline const char_type* byte_to_str(uint8_t v,char_type* to) noexcept {
		return generic_uint_cast<uint8_t,char_type>::utoa(v,to);
	}

	static inline uint8_t str_to_byte(const char_type* str,const char_type** endp) noexcept {
		return  generic_uint_cast<uint8_t,char_type>::str_to_u(str, endp);
	}

	static inline const char_type* small_to_str(int8_t v,char_type* to) noexcept {
		return generic_int_cast<int8_t,char_type>::itoa(v,to);
	}

	static inline int8_t str_to_small(const char_type* str,const char_type** endptr) noexcept {
		return generic_int_cast<int8_t,char_type>::str_to_i(str,endptr);
	}

	static inline const char_type* word_to_str(uint16_t v,char_type* to) noexcept {
		return generic_uint_cast<uint16_t,char_type>::utoa(v,to);
	}

	static inline uint16_t str_to_word(const char_type* str,const char_type** endp) noexcept {
		return generic_uint_cast<uint32_t,char_type>::str_to_u(str,endp);
	}

	static inline const char_type* short_to_str(int16_t v,char_type* to) noexcept {
		return generic_int_cast<int16_t,char_type>::itoa(v,to);
	}

	static inline int16_t str_to_short(const char_type* str,const char_type** endp) noexcept {
		return generic_int_cast<int16_t,char_type>::str_to_i(str,endp);
	}

	static inline const char_type* dword_to_str(uint32_t v,char_type* to) noexcept {
		return generic_uint_cast<uint32_t,char_type>::utoa(v,to);
	}

	static inline uint32_t str_to_dword(const char_type* str,const char_type** endp) noexcept {
		return generic_uint_cast<uint32_t,char_type>::str_to_u(str,endp);
	}

	static inline const char_type* int_to_str(int32_t v,char_type* to) noexcept {
		return generic_int_cast<int32_t,char_type>::itoa(v,to);
	}

	static inline int32_t str_to_int(const char_type* str,const char_type** endp) noexcept {
		return generic_int_cast<uint32_t,char_type>::str_to_i(str,endp);
	}

	static inline const char_type* qword_to_str(uint64_t v,char_type* to) noexcept {
		return generic_uint_cast<uint64_t,char_type>::utoa(v,to);
	}

	static inline uint64_t str_to_qword(const char_type* str,const char_type** endp) noexcept {
		return generic_uint_cast<uint64_t,char_type>::str_to_u(str,endp);
	}

	static inline const char_type* long_to_str(int64_t v,char_type* to) noexcept {
		return generic_int_cast<int64_t,char_type>::itoa(v,to);
	}

	static inline int64_t str_to_long(const char_type* str,const char_type** endp) noexcept {
		return generic_int_cast<int64_t,char_type>::str_to_i(str,endp);
	}

	static inline const char_type* float_to_str(float v, char_type* to) noexcept {
		char buff[ FLOAT_MAX_DIGITS + 1 ];
#ifdef __GNUG__
		__builtin_snprintf(buff, FLOAT_MAX_DIGITS, "%G", v);
#else
		std::sprintf(buff, "%G", v);
#endif // __GNUG__
		return move_digits<char_type>::move( to, buff);
	}

	static inline float str_to_float(const char_type* str,const char_type** endptr) noexcept {
		char buff[FLOAT_MAX_DIGITS + 1];
		move_digits<char_type>::move( buff, str);
		char *ep;
		float ret = std::strtof(buff, &ep);
		if(nullptr != ep)
			*endptr = nullptr;
		else
			*endptr = const_cast<char_type*>(str + memory_traits::distance(buff,ep));
		return ret;
	}

	static inline const char_type* double_to_str(double v, char_type* to) noexcept {
		char buff[DOUBLE_MAX_DIGITS+1];
		io_memset(buff, '\0', DOUBLE_MAX_DIGITS+1);
#ifdef __GNUG__
		__builtin_snprintf(buff, DOUBLE_MAX_DIGITS, "%G", v);
#else
		std::sprintf(buff, "%G", v);
#endif // __GNUG__
		return move_digits<char_type>::move( to, buff);
	}

	static inline double str_to_double(const char_type* str,const char_type** endptr) noexcept {
		char buff[DOUBLE_MAX_DIGITS+1] = {'\0'};
		move_digits<char_type>::move( buff, str);
		char *ep;
		double ret = std::strtod(buff, &ep);
		if(nullptr != ep)
			*endptr = nullptr;
		else
			*endptr = const_cast<char_type*>(str + memory_traits::distance(buff,ep));
		return ret;
	}

	static inline const char_type* longdouble_to_str(long double v,char_type* to) noexcept {
		char buff[LONG_DOUBLE_MAX_DIGITS+1] = {'\0'};
#ifdef __GNUG__
		__builtin_snprintf(buff, LONG_DOUBLE_MAX_DIGITS, "%G", v);
#else
		std::sprintf(buff, "%G", v);
#endif // __GNUG__
		return move_digits<char_type>::move( to, buff);
	}

	static inline long double str_to_longdouble(const char_type* str,char_type* const endp) noexcept {
		char buff[LONG_DOUBLE_MAX_DIGITS+1];
		move_digits<char_type>::move( buff, str);
		char *ep;
		long double ret = std::strtold(buff, &ep);
		if(nullptr != ep)
			*endp = nullptr;
		else
			*endp = const_cast<char_type*>(str + memory_traits::distance(buff,ep));
		return ret;
	}

	static inline const char_type* boolean_to_str(bool value, char_type* to) noexcept {
		if(value)
			char_traits::move(to, strings_traits::true_str, 5);
		else
			char_traits::move(to, strings_traits::false_str, 6);
		return to;
	}

	static inline bool str_to_boolean(const char_type* from,const char_type* * endp) noexcept {
		char_type tmp[5];
		for(uint8_t i=0; i < 5; i++)
			tmp[i] = char_traits::to_char_type( io_tolower( char_traits::to_int_type(from[i] ) ) );
		bool ret = 0 == char_traits::compare(tmp, strings_traits::true_str, 4);
		if(ret) {
		  	*endp = const_cast<char_type*>(from + 4);
		} else if( 0 == char_traits::compare(tmp, strings_traits::false_str,5) ) {
			*endp = const_cast<char_type*>(from + 5);
		} else {
			*endp = nullptr;
		}
		return ret;
	}

};

typedef lexical_cast<char> char8_lexical_cast;
typedef lexical_cast<uint16_t> char16_lexical_cast;
typedef lexical_cast<uint32_t> char32_lexical_cast;
typedef lexical_cast<wchar_t> wchar_lexical_cast;

template<typename T, class __lex_cast = char8_lexical_cast>
class lexical_cast_traits {
};

template<class __lex_cast>
class lexical_cast_traits<uint8_t,__lex_cast> {
private:
	typedef __lex_cast lext_cast;
public:
	typedef typename lext_cast::char_type char_type;
	static constexpr uint8_t max_str_len = int_limits<uint8_t>::max_str_len;
	static inline void  to_string(uint8_t byte, char_type* to) noexcept {
		return lext_cast::byte_to_str(byte, to);
	}
	static inline uint8_t from_string(const char_type* from) noexcept {
		return lext_cast::str_to_byte(from);
	}
};


#ifndef _MSC_VER
template<class __lex_cast>
class lexical_cast_traits<int8_t,__lex_cast> {
private:
	typedef __lex_cast lext_cast;

public:

	typedef typename lext_cast::char_type char_type;

	static constexpr std::size_t max_str_len = int_limits<int8_t>::max_str_len;

	static void to_string(const int8_t small, char_type* to) noexcept {
		return lext_cast::small_to_str(small, to);
	}

	static int8_t from_string(const char_type* from) noexcept {
		return lext_cast::str_to_small(from);
	}

};
#endif

template<class __lex_cast>
class lexical_cast_traits<uint16_t,__lex_cast> {
private:
	typedef __lex_cast lext_cast;
public:
	typedef typename lext_cast::char_type char_type;

	static constexpr std::size_t max_str_len = int_limits<int8_t>::max_str_len;

	static  void to_string(const uint16_t word, char_type* to) noexcept {
		return lext_cast::word_to_str(word, to);
	}

	static uint16_t from_string(const char_type* from) noexcept {
		return lext_cast::str_to_word(from);
	}

};

#ifndef _MSC_VER
template<class __lex_cast>
class lexical_cast_traits<int16_t,__lex_cast> {
private:
	typedef __lex_cast lext_cast;
public:
	typedef typename lext_cast::char_type char_type;
	static constexpr uint8_t max_str_len = int_limits<int16_t>::max_str_len;
	static void to_string(const int16_t small, char_type* to) noexcept {
		return lext_cast::short_to_str(small, to);
	}
	static uint16_t from_string(const char_type* from) noexcept {
		return lext_cast::str_to_short(from);
	}
};
#endif


template<class __lex_cast>
class lexical_cast_traits<uint32_t,__lex_cast> {
private:
	typedef __lex_cast lext_cast;
public:
	typedef typename lext_cast::char_type char_type;
	static constexpr uint8_t max_str_len = int_limits<uint32_t>::max_str_len;
	static inline void to_string(int32_t dword, char_type* to) noexcept {
		return lext_cast::dword_to_str(dword, to);
	}
	static inline uint32_t from_string(const char_type* from) noexcept {
		return lext_cast::str_to_dword(from);
	}
};

template<class __lex_cast>
class lexical_cast_traits<int32_t,__lex_cast> {
private:
	typedef __lex_cast lext_cast;
public:
	typedef typename lext_cast::char_type char_type;
	static constexpr uint8_t max_str_len = int_limits<int32_t>::max_str_len;
	static inline void to_string(int32_t dword, char_type* to) noexcept {
		return lext_cast::int_to_str(dword, to);
	}
	static inline uint32_t from_string(const char_type* from) noexcept {
		return lext_cast::str_to_int(from);
	}
};

template<class __lex_cast>
class lexical_cast_traits<uint64_t,__lex_cast> {
private:
	typedef __lex_cast lext_cast;
public:
	typedef typename lext_cast::char_type char_type;
	static constexpr uint8_t max_str_len = int_limits<uint64_t>::max_str_len;
	static inline void to_string(uint64_t qword, char_type* to) noexcept {
		return lext_cast::qword_to_str(qword, to);
	}
	static inline uint64_t from_string(const char_type* from,const char_type** endp) noexcept {
		return lext_cast::str_to_qword(from,endp);
	}
	static inline uint64_t from_string(const char_type* from) noexcept {
		const char_type* endp[1];
		return lext_cast::str_to_qword(from,endp);
	}
};

template<class __lex_cast>
class lexical_cast_traits<int64_t,__lex_cast> {
private:
	typedef __lex_cast lext_cast;
public:
	typedef typename lext_cast::char_type char_type;
	static constexpr uint8_t max_str_len = int_limits<int64_t>::max_str_len;
	static inline void to_string(int64_t llong, char_type* to) noexcept {
		return lext_cast::long_to_str(llong, to);
	}
	static inline int64_t from_string(const char_type* from,const char_type** endp) noexcept {
		return lext_cast::str_to_long(from,endp);
	}
};

template<class __lex_cast>
class lexical_cast_traits<float,__lex_cast> {
private:
	typedef __lex_cast lext_cast;
public:
	typedef typename lext_cast::char_type char_type;
	static constexpr uint8_t max_str_len = 32;
	static inline void to_string(float real, char_type* to) noexcept {
		return lext_cast::float_to_str(real, to);
	}
	static inline float from_string(const char_type* from,const char_type** endp) noexcept {
		return lext_cast::str_to_float(from,endp);
	}
};

template<class __lex_cast>
class lexical_cast_traits<double,__lex_cast> {
private:
	typedef __lex_cast lext_cast;
public:
	typedef typename lext_cast::char_type char_type;
	static constexpr uint8_t max_str_len = 64;
	static inline void to_string(double deximal, char_type* to) noexcept {
		return lext_cast::double_to_str(deximal, to);
	}
	static inline double from_string(const char_type* from,const char_type** endp) noexcept {
		return lext_cast::str_to_dobule(from,endp);
	}
};

template<class __lex_cast>
class lexical_cast_traits<long double,__lex_cast> {
private:
	typedef __lex_cast lext_cast;
public:
	typedef typename lext_cast::char_type char_type;
	static constexpr uint8_t max_str_len = 128;
	static inline void to_string(long double big_deximal, char_type* to) noexcept {
		return lext_cast::longdouble_to_str(big_deximal, to);
	}
	static inline long double from_string(const char_type* from,const char_type** endp) noexcept {
		return lext_cast::str_to_longdouble(from,endp);
	}
};

template<class __lex_cast>
class lexical_cast_traits<bool,__lex_cast> {
private:
	typedef __lex_cast lext_cast;
public:
	typedef typename lext_cast::char_type char_type;
	static constexpr uint8_t max_str_len = 6;
	static inline void to_string(bool __b, char_type* to) noexcept {
		return lext_cast::boolean_to_str(__b, to);
	}
	static inline bool from_string(const char_type* from,const char_type** endp) noexcept {
		return lext_cast::str_to_boolean(from,endp);
	}
	static inline bool from_string(const char_type* from) noexcept {
		const char_type* endp[1];
		return lext_cast::str_to_boolean(from,endp);
	}
};

} // namesapce xml

} // namespace io


#endif // __IO_XML_LEXCAST_HPP_INCLUDED__
