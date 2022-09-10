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
#ifndef __IO_CHAR_CAST_INCLUDED__
#define __IO_CHAR_CAST_INCLUDED__

#include "config.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <stdint.h>
#include <limits>
#include <cstdlib>
#include <ctime>
#include <time.h>
#include <chrono>

#include "conststring.hpp"
#include "type_traits_ext.hpp"

namespace io {

struct to_chars_result {
	char* ptr;
	std::errc ec;
	friend bool operator==(const to_chars_result& lhs, const to_chars_result& rhs) noexcept
	{
		return lhs.ptr == rhs.ptr && lhs.ec == rhs.ec;
	};
};

struct from_chars_result {
	const char* ptr;
	std::errc ec;
	friend bool operator==(const from_chars_result& lhs, const from_chars_result& rhs) noexcept
	{
		return lhs.ptr == rhs.ptr && lhs.ec == rhs.ec;
	}
};

namespace detail {

#ifdef _MSC_VER

IO_PUBLIC_SYMBOL char* uintmax_to_chars_reverse(char* const last, uintmax_t value) noexcept;

IO_PUBLIC_SYMBOL from_chars_result unsigned_from_chars(const char* first, const char* last, uintmax_t& value) noexcept;
IO_PUBLIC_SYMBOL from_chars_result signed_from_chars(const char* first, const char* last, intmax_t& value) noexcept;

IO_PUBLIC_SYMBOL to_chars_result float_to_chars(char* const first, char* const last, float value) noexcept;
IO_PUBLIC_SYMBOL to_chars_result float_to_chars(char* const first, char* const last, double value) noexcept;
IO_PUBLIC_SYMBOL to_chars_result float_to_chars(char* const first, char* const last, const long double& value) noexcept;

IO_PUBLIC_SYMBOL from_chars_result float_from_chars(const char* first, const char* last, float& value) noexcept;
IO_PUBLIC_SYMBOL from_chars_result float_from_chars(const char* first, const char* last, double& value) noexcept;
IO_PUBLIC_SYMBOL from_chars_result float_from_chars(const char* first, const char* last, long double& value) noexcept;

IO_PUBLIC_SYMBOL to_chars_result time_to_chars(char* first, char* last, const char* format, const std::time_t& value) noexcept;
IO_PUBLIC_SYMBOL to_chars_result time_from_chars(const char* first,const char* last, const char* format, std::time_t& value) noexcept;

#else

char* IO_PUBLIC_SYMBOL uintmax_to_chars_reverse(char* const last, uintmax_t value) noexcept;

from_chars_result IO_PUBLIC_SYMBOL unsigned_from_chars(const char* first, const char* last, uintmax_t& value) noexcept;
from_chars_result IO_PUBLIC_SYMBOL signed_from_chars(const char* first, const char* last, intmax_t& value) noexcept;

to_chars_result IO_PUBLIC_SYMBOL float_to_chars(char* const first, char* const last, float value) noexcept;
to_chars_result IO_PUBLIC_SYMBOL float_to_chars(char* const first, char* const last, double value) noexcept;
to_chars_result IO_PUBLIC_SYMBOL float_to_chars(char* const first, char* const last, const long double& value) noexcept;

from_chars_result IO_PUBLIC_SYMBOL float_from_chars(const char* first, const char* last, float& value) noexcept;
from_chars_result IO_PUBLIC_SYMBOL float_from_chars(const char* first, const char* last, double& value) noexcept;
from_chars_result IO_PUBLIC_SYMBOL float_from_chars(const char* first, const char* last, long double& value) noexcept;

to_chars_result IO_PUBLIC_SYMBOL time_to_chars(char* first, char* last, const char* format, const std::time_t& value) noexcept;
to_chars_result IO_PUBLIC_SYMBOL time_from_chars(const char* first,const char* last, const char* format, std::time_t& value) noexcept;

#endif // _MSC_VER


} // namespace detail


#ifdef IO_HAS_CONNCEPTS
template<typename T>
	requires( is_unsigned_integer_v<T> )
#else
template<
	typename T,
	typename std::enable_if<
		is_unsigned_integer<T>::value
		>::type* = nullptr
	>
#endif // IO_HAS_CONNCEPTS
to_chars_result to_chars(char* const first, char* const last, T value) noexcept
{
	to_chars_result ret = {nullptr, {}};
	if( first >= last ) {
		ret.ec = std::errc::no_buffer_space;
	}
	else if(0 == value) {
		*first = '0';
		ret.ptr = first + 1;
	}
	else {
		static constexpr std::size_t buff_size = 32;
		char tmp[ buff_size ] = { '\0' };
		char *s = detail::uintmax_to_chars_reverse( (tmp + buff_size-1), static_cast<uintmax_t>(value) );
		const std::size_t len = memory_traits::distance( s, (tmp + buff_size) );
		if( len > memory_traits::distance(first,last)) {
			ret.ec = std::errc::no_buffer_space;
		}
		else {
			io_memmove(first, s, len);
			ret.ptr = first + len;
		}
	}
	return ret;
}

#ifdef IO_HAS_CONNCEPTS
template<typename T>
requires( is_signed_integer_v<T> )
#else
template<
	typename T,
	typename std::enable_if<
		is_signed_integer<T>::value
		>::type* = nullptr
	>
#endif
to_chars_result to_chars(char* const first, char* const last, T value) noexcept
{
	to_chars_result ret = {nullptr, {}};
	if( first >= last ) {
		ret.ec = std::errc::no_buffer_space;
	} else {
		static constexpr std::size_t buff_size = 32;

		char tmp[ buff_size ] = { '\0' };
		intmax_t sv = static_cast<intmax_t>(value);
		uintmax_t uv = (sv < 0) ? static_cast<uintmax_t>(~sv + 1) : static_cast<uintmax_t>(sv);

		char *s = detail::uintmax_to_chars_reverse( (tmp + buff_size-1), uv );
		if(sv < 0) {
			--s;
			*s = '-';
		}

		const std::size_t len =  memory_traits::distance(s, (tmp + buff_size) );
		const std::size_t out_buff = memory_traits::distance(first,last);
		if( len > out_buff ) {
			ret.ec = std::errc::no_buffer_space;
		}
		else {
			io_memmove(first, s, len);
			ret.ptr = first + len;
		}
	}
	return ret;
}


#ifdef IO_HAS_CONNCEPTS
template<typename T>
requires( std::is_floating_point_v<T> )
#else
template<
	typename T,
	typename std::enable_if<
		std::is_floating_point<T>::value
		>::type* = nullptr
	>
#endif // IO_HAS_CONNCEPTS
to_chars_result to_chars(char* const first, char* const last,const T& value) noexcept
{
	return detail::float_to_chars(first, last, value);
}

#ifdef IO_HAS_CONNCEPTS
template<typename _Tp>
requires( is_unsigned_integer_v<_Tp> )
#else
template<
	typename _Tp,
	typename std::enable_if<
		is_unsigned_integer<_Tp>::value
		>::type* = nullptr
	>
#endif // IO_HAS_CONNCEPTS
from_chars_result from_chars(const char* first, const char* last, _Tp& value) noexcept
{
	std::size_t v;
	from_chars_result ret = detail::unsigned_from_chars(first, last, v);
	constexpr _Tp max_value = (std::numeric_limits< _Tp > ::max)();
	if(max_value < v ) {
		ret.ptr = nullptr;
		ret.ec = std::errc::result_out_of_range;
	}
	value = static_cast<_Tp>(v);
	return ret;
}

#ifdef IO_HAS_CONNCEPTS
template<typename _Tp>
requires( is_signed_integer_v<_Tp> )
#else
template<
	typename _Tp,
	typename std::enable_if<
		is_signed_integer<_Tp>::value
		>::type* = nullptr
	>
#endif // IO_HAS_CONNCEPTS
from_chars_result from_chars(const char* first, const char* last, _Tp& value) noexcept
{
	intmax_t v;
	from_chars_result ret = detail::signed_from_chars(first, last, v);
	const _Tp min_value = (std::numeric_limits< _Tp >::min)();
	const _Tp max_value = (std::numeric_limits< _Tp >::max)();
	if( v < min_value  || v > max_value ) {
		ret.ptr = nullptr;
		ret.ec = std::errc::result_out_of_range;
	}
	value = static_cast<_Tp>(v);
	return ret;
}

#ifdef IO_HAS_CONNCEPTS
template<typename T>
requires( std::is_floating_point_v<T> )
#else
template<
	typename T,
	typename std::enable_if<
		std::is_floating_point<T>::value
		>::type* = nullptr
	>
#endif // IO_HAS_CONNCEPTS
from_chars_result from_chars(const char* first,const char* last, T& value) noexcept
{
	return detail::float_from_chars(first, last, value);
}

enum class str_bool_format {
	true_false,
	yes_no
};

#ifdef _MSC_VER
to_chars_result IO_PUBLIC_SYMBOL to_chars(char* first, char* last, bool value, str_bool_format fmt = str_bool_format::true_false) noexcept;
from_chars_result IO_PUBLIC_SYMBOL from_chars(const char* first,const char* last, bool& value) noexcept;
#else
IO_PUBLIC_SYMBOL to_chars_result to_chars(char* first, char* last, bool value, str_bool_format fmt = str_bool_format::true_false) noexcept;
IO_PUBLIC_SYMBOL from_chars_result from_chars(const char* first,const char* last, bool& value) noexcept;
#endif // _MSC_VER

#ifdef IO_HAS_CONNCEPTS
template<typename T>
requires( is_signed_integer_v<T> || is_unsigned_integer_v<T> || std::is_floating_point_v<T> || std::is_same_v<bool,T> )
#else
template<
	typename T,
	typename std::enable_if<
		is_signed_integer<T>::value ||
		is_unsigned_integer<T>::value ||
		std::is_floating_point<T>::value ||
		std::is_same<bool,T>::value
		>::type* = nullptr
	>
#endif // IO_HAS_CONNCEPTS
inline void from_string(std::error_code& ec, const io::const_string& str, T& value) noexcept
{
	auto fch_ret = from_chars( str.data(), str.data()+str.size(), value);
	if(nullptr == fch_ret.ptr)
		ec = std::make_error_code( fch_ret.ec );
}

#ifdef IO_HAS_CONNCEPTS
template<typename T>
requires( is_signed_integer_v<T> || is_unsigned_integer_v<T> || std::is_floating_point_v<T> || std::is_same_v<bool,T>)
#else
template<
	typename T,
	typename std::enable_if<
		is_signed_integer<T>::value ||
		is_unsigned_integer<T>::value ||
		std::is_floating_point<T>::value ||
		std::is_same<bool,T>::value
		>::type* = nullptr
	>
#endif // IO_HAS_CONNCEPTS
inline void from_string(const io::const_string& str, T& value)
{
	std::error_code ec;
	from_string(ec, str, value);
	io::check_error_code(ec);
}

#ifdef IO_HAS_CONNCEPTS
template<typename T>
requires( is_signed_integer_v<T> || is_unsigned_integer_v<T> || std::is_floating_point_v<T> )
#else
template<
	typename T,
	typename std::enable_if<
		is_signed_integer<T>::value ||
		is_unsigned_integer<T>::value ||
		std::is_floating_point<T>::value
		>::type* = nullptr
	>
#endif // IO_HAS_CONNCEPTS
inline io::const_string to_string(std::error_code& ec, const T value) noexcept
{
	static constexpr std::size_t buff_size = 32;
	char tmp[buff_size] = {'\0'};
	auto ret = io::to_chars(tmp, (tmp+buff_size), value);
	if( 0 != static_cast<unsigned int>(ret.ec) ) {
 		ec = std::make_error_code( ret.ec );
		return io::const_string();
	}
	return io::const_string(tmp);
}

#ifdef _MSC_VER
IO_PUBLIC_SYMBOL io::const_string to_string(std::error_code& ec, const bool value, str_bool_format fmt = str_bool_format::true_false) noexcept;
#else
io::const_string IO_PUBLIC_SYMBOL to_string(std::error_code& ec, const bool value, str_bool_format fmt = str_bool_format::true_false) noexcept;
#endif // _MSC_VER

inline io::const_string to_string(const bool value, str_bool_format fmt = str_bool_format::true_false)
{
	std::error_code ec;
	auto ret = to_string(ec, value, fmt);
	io::check_error_code(ec);
	return ret;
}

} // namespace io

#endif // __IO_CHAR_CAST_INCLUDED__
