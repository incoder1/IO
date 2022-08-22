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

from_chars_result IO_PUBLIC_SYMBOL unsigned_from_chars(const char* first, const char* last, std::size_t& value) noexcept;
from_chars_result IO_PUBLIC_SYMBOL signed_from_chars(const char* first, const char* last, ssize_t& value) noexcept;

to_chars_result IO_PUBLIC_SYMBOL float_to_chars(char* const first, char* const last, float value) noexcept;
to_chars_result IO_PUBLIC_SYMBOL float_to_chars(char* const first, char* const last, double value) noexcept;
to_chars_result IO_PUBLIC_SYMBOL float_to_chars(char* const first, char* const last, const long double& value) noexcept;

from_chars_result IO_PUBLIC_SYMBOL float_from_chars(const char* first, const char* last, float& value) noexcept;
from_chars_result IO_PUBLIC_SYMBOL float_from_chars(const char* first, const char* last, double& value) noexcept;
from_chars_result IO_PUBLIC_SYMBOL float_from_chars(const char* first, const char* last, long double& value) noexcept;

to_chars_result IO_PUBLIC_SYMBOL time_to_chars(char* first, char* last, const char* format, const std::time_t& value) noexcept;
to_chars_result IO_PUBLIC_SYMBOL time_from_chars(const char* first,const char* last, const char* format, std::time_t& value) noexcept;


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
	to_chars_result ret = {nullptr, std::errc()};
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
		char *s = tmp + buff_size-1;
		std::size_t len = 0;
		do {
			++len;
			*s = '0' + (value % 10);
			value /= 10;
			--s;
		}
		while( 0 != value );
		if( len > memory_traits::distance(first,last)) {
			ret.ec = std::errc::no_buffer_space;
			len = memory_traits::distance(first,last);
		}
		else {
			io_memmove(first, s+1, len);
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
to_chars_result to_chars(char* const first, char* const last,const T value) noexcept
{
	to_chars_result ret = {nullptr, std::errc()};
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
		char *s = tmp + buff_size-1;
		typedef typename std::make_unsigned<T>::type uint_type;
		uint_type uv = (value < 0) ? static_cast<uint_type>(-value) : static_cast<uint_type>(value);
		std::size_t len = 0;
		do {
			++len;
			*s = '0' + (uv % 10);
			uv /= 10;
			--s;
		}
		while( 0 != uv );
		if(value < 0) {
			*s = '-';
			++len;
			--s;
		}
		if( len > memory_traits::distance(first,last)) {
			ret.ec = std::errc::no_buffer_space;
			len = memory_traits::distance(first,last);
		}
		else {
			io_memmove(first, s+1, len);
			ret.ptr = first + len;
		}
	}
	return ret;
}

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
from_chars_result from_chars(const char* first, const char* last, T& value) noexcept
{
	std::size_t v;
	from_chars_result ret = detail::unsigned_from_chars(first, last, v);
	if( std::numeric_limits<T>::max() < v ) {
		ret.ptr = nullptr;
		ret.ec = std::errc::result_out_of_range;
	}
	value = static_cast<T>(v);
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
#endif // IO_HAS_CONNCEPTS
from_chars_result from_chars(const char* first, const char* last, T& value) noexcept
{
	ssize_t v;
	from_chars_result ret = detail::signed_from_chars(first, last, v);
	typedef std::numeric_limits<T> limits;
	if( v > limits::max() || v < limits::min() ) {
		ret.ptr = nullptr;
		ret.ec = std::errc::result_out_of_range;
	}
	return static_cast<T>(v);
}

inline to_chars_result to_chars(char* const first, char* const last, float value) noexcept
{
	return detail::float_to_chars(first, last, value);
}

inline to_chars_result to_chars(char* const first, char* const last, double value) noexcept
{
	return detail::float_to_chars(first, last, value);
}

inline to_chars_result to_chars(char* const first, char* const last, const long double& value) noexcept
{
	return detail::float_to_chars(first, last, value);
}

// inline to_chars_result to_chars(char )


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
to_chars_result from_chars(const char* first,const char* last, T& value) noexcept
{
	return detail::float_from_chars(first, last, value);
}

enum class str_bool_format {
	true_false,
	yes_no
};

to_chars_result IO_PUBLIC_SYMBOL to_chars(char* first, char* last, bool value, str_bool_format fmt = str_bool_format::true_false) noexcept;
from_chars_result IO_PUBLIC_SYMBOL from_chars(const char* first,const char* last, bool& value) noexcept;

#ifdef IO_HAS_CONNCEPTS
template<typename T>
requires( std::is_integral_v<T> || std::is_floating_point_v<T> )
#else
template<
	typename T,
	typename std::enable_if<
		std::is_integral<T>::value ||
		std::is_floating_point<T>::value
		>::type* = nullptr
	>
#endif // IO_HAS_CONNCEPTS
inline void from_string(std::error_code& ec, const io::const_string& str, T& value) noexcept
{
	io::from_chars_result fch_ret = io::from_chars( str.data(), str.data()+str.size(), value);
	if(nullptr == fch_ret.ptr)
		ec = std::make_error_code( fch_ret.ec );
}

#ifdef IO_HAS_CONNCEPTS
template<typename T>
requires( std::is_integral_v<T> || std::is_floating_point_v<T> )
#else
template<
	typename T,
	typename std::enable_if<
		std::is_integral<T>::value ||
		std::is_floating_point<T>::value
		>::type* = nullptr
	>
#endif
inline void from_string(const io::const_string& str, T& value)
{
	std::error_code ec;
	from_string(ec, str, value);
	io::check_error_code(ec);
}

#ifdef IO_HAS_CONNCEPTS
template<typename T>
requires( is_signed_integer_v<T> || is_unsigned_integer_v<T>  )
#else
template<
	typename T,
	typename std::enable_if<
		is_signed_integer<T>::value ||
		is_unsigned_integer<T>::value
		>::type* = nullptr
	>
#endif // IO_HAS_CONNCEPTS
inline io::const_string to_string(std::error_code& ec, T value) noexcept
{
	typedef std::numeric_limits<T> limits;
	constexpr std::size_t buff_size = limits::digits10 + 1;
	char tmp[ buff_size ] = {'\0'};
	to_chars_result tch_ret = io::to_chars(tmp, buff_size, value);
	if(nullptr == tch_ret.ptr) {
		ec = std::make_error_code( tch_ret.ec );
		return io::const_string();
	}
	return io::const_string(tmp);
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
inline io::const_string to_string(std::error_code& ec, const T& value) noexcept
{
	typedef std::numeric_limits<T> limits;
	constexpr std::size_t buff_size = limits::digits10 + 1;
	char tmp[ buff_size ] = {'\0'};
	to_chars_result tch_ret = detail::float_to_chars(tmp, tmp+buff_size, value);
	if(nullptr == tch_ret.ptr) {
		ec = std::make_error_code( tch_ret.ec );
		return io::const_string();
	}
	return io::const_string(tmp);
}

} // namespace io

#endif // __IO_CHAR_CAST_INCLUDED__
