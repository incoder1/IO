/*
 *
 * Copyright (c) 2016-2021
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#include "stdafx.hpp"

#include "char_cast.hpp"

namespace io {

namespace detail {

static char* fmt_true_false(char* to, std::size_t buf_size, bool value) noexcept
{
    if(value && buf_size >= 4) {
        io_memmove(to, "true", 4);
        return to+4;
    }
    else if(!value && buf_size >= 5) {
        io_memmove(to, "false", 5);
        return to+5;
    }
    return nullptr;
}

static char* fmt_yes_no(char* to, std::size_t buf_size, bool value) noexcept
{
    if(value && buf_size >= 3) {
        io_memmove(to, "yes", 3);
        return to+4;
    }
    else if(!value && buf_size >= 2) {
        io_memmove(to, "no", 2);
        return to+5;
    }
    return nullptr;
}

from_chars_result IO_PUBLIC_SYMBOL unsigned_from_chars(const char* first, const char* last, std::size_t& value) noexcept
{
    from_chars_result ret = {nullptr, std::errc()};
    if(0 == memory_traits::distance(first,last) ) {
        ret.ec = std::errc::invalid_argument;
    }
    else {
        char *endp;
#	ifdef IO_CPU_BITS_64
        value = std::strtoull(first, &endp, 10);
#	else
        value = std::strtoul(first, &endp, 10);
#	endif // IO_CPU_BITS_64
        typedef std::numeric_limits<std::size_t> limits;
        if ( (limits::max() == value) && (ERANGE == errno) ) {
            ret.ptr = nullptr;
            ret.ec = std::errc::result_out_of_range;
        }
        else {
            ret.ptr = endp;
        }
    }
    return ret;
}

from_chars_result IO_PUBLIC_SYMBOL signed_from_chars(const char* first, const char* last, ssize_t& value) noexcept
{
    from_chars_result ret = {nullptr, std::errc()};
    if(0 == memory_traits::distance(first,last) ) {
        ret.ec = std::errc::invalid_argument;
    }
    else {
        char *endp;
#	ifdef IO_CPU_BITS_64
        value = std::strtoll(first, &endp, 10);
#	else
        value = std::strtol(first, &endp, 10);
#	endif // IO_CPU_BITS_64
        typedef std::numeric_limits<ssize_t> limits;
        if ( ( (limits::max() == value) || (limits::min() == value) )  && (ERANGE == errno) ) {
            ret.ptr = nullptr;
            ret.ec = std::errc::result_out_of_range;
        }
        else {
            ret.ptr = endp;
        }
    }
    return ret;
}


template<typename T>
static constexpr unsigned int float_max_digits() noexcept
{
    typedef std::numeric_limits<T> limits_type;
    return 3 + limits_type::digits - limits_type::min_exponent;
}

static constexpr std::size_t FLOAT_MAX_DIGITS = float_max_digits<float>() + 1;
static constexpr std::size_t DOUBLE_MAX_DIGITS = float_max_digits<double>() + 1;
static constexpr std::size_t LONG_DOUBLE_MAX_DIGITS = float_max_digits<long double>() + 1;

to_chars_result IO_PUBLIC_SYMBOL float_to_chars(char* first, char* last, float value) noexcept
{

    to_chars_result ret = {nullptr, std::errc()};
    std::size_t buf_size = memory_traits::distance(first,last);
    if( 0 == buf_size ) {
        ret.ec = std::errc::no_buffer_space;
    }
    else {
        char buff[ FLOAT_MAX_DIGITS ] = {'\0'};
#ifdef __GNUG__
        __builtin_snprintf(buff, FLOAT_MAX_DIGITS, "%G", value);
#else
        std::snprintf(buff, FLOAT_MAX_DIGITS, "%G", value);
#endif // __GNUG__
        std::size_t len = io_strlen(buff);
        if( len > buf_size ) {
            ret.ec = std::errc::no_buffer_space;
        }
        else {
            io_memmove(first, buff, len);
            ret.ptr = first + len;
        }
    }
    return ret;
}

to_chars_result IO_PUBLIC_SYMBOL float_to_chars(char* first, char* last, double value) noexcept
{
    to_chars_result ret = {nullptr, std::errc()};
    std::size_t buf_size = memory_traits::distance(first,last);
    if( 0 == buf_size ) {
        ret.ec = std::errc::no_buffer_space;
    }
    else {
        char buff[ DOUBLE_MAX_DIGITS ] = {'\0'};
#ifdef __GNUG__
        __builtin_snprintf(buff, DOUBLE_MAX_DIGITS, "%G", value);
#else
        std::snprintf(buff, DOUBLE_MAX_DIGITS, "%G", value);
#endif // __GNUG__
        std::size_t len = io_strlen(buff);
        if( len > buf_size ) {
            ret.ec = std::errc::no_buffer_space;
        }
        else {
            io_memmove(first, buff, len);
            ret.ptr = first + len;
        }
    }
    return ret;
}

to_chars_result IO_PUBLIC_SYMBOL float_to_chars(char* first, char* last, const long double& value) noexcept
{
    to_chars_result ret = {nullptr, std::errc()};
    std::size_t buf_size = memory_traits::distance(first,last);
    if( 0 == buf_size ) {
        ret.ec = std::errc::no_buffer_space;
    }
    else {
        char buff[ LONG_DOUBLE_MAX_DIGITS ] = {'\0'};
#if defined(__GNUG__) && !defined(__MINGW32__) && !defined(__MINGW64__)
        __builtin_snprintf(buff, LONG_DOUBLE_MAX_DIGITS, "%LG", value);
#else
        std::snprintf(buff, LONG_DOUBLE_MAX_DIGITS, "%LG", value);
#endif // __GNUG__
        std::size_t len = io_strlen(buff);
        if( len > buf_size ) {
            ret.ec = std::errc::no_buffer_space;
        }
        else {
            io_memmove(first, buff, len);
            ret.ptr = first + len;
        }
    }
    return ret;
}

from_chars_result IO_PUBLIC_SYMBOL float_from_chars(const char* first, const char* last, float& value) noexcept
{
    from_chars_result ret = {nullptr, std::errc()};
    if(0 == memory_traits::distance(first,last) ) {
        ret.ec = std::errc::invalid_argument;
    }
    else {
        char *endp;
        value = std::strtof(first,&endp);
        if ( nullptr == endp ) {
            ret.ptr = nullptr;
            ret.ec = std::errc::argument_out_of_domain;
        }
        else {
			ret.ptr = endp;
        }
    }
    return ret;
}

from_chars_result IO_PUBLIC_SYMBOL float_from_chars(const char* first, const char* last, double& value) noexcept
{
    from_chars_result ret = {nullptr, std::errc()};
    if(0 == memory_traits::distance(first,last) ) {
        ret.ec = std::errc::invalid_argument;
    }
    else {
        char *endp;
        value = std::strtod(first,&endp);
        if ( nullptr == endp ) {
            ret.ptr = nullptr;
            ret.ec = std::errc::argument_out_of_domain;
        }
        else {
			ret.ptr = endp;
        }
    }
    return ret;
}

from_chars_result IO_PUBLIC_SYMBOL float_from_chars(const char* first, const char* last, long double& value) noexcept
{
    from_chars_result ret = {nullptr, std::errc()};
    if(0 == memory_traits::distance(first,last) ) {
        ret.ec = std::errc::invalid_argument;
    }
    else {
        char *endp;
        value = std::strtold(first,&endp);
        if ( nullptr == endp ) {
            ret.ptr = nullptr;
            ret.ec = std::errc::argument_out_of_domain;
        }
        else {
			ret.ptr = endp;
        }
    }
    return ret;
}

} // namespace detail


to_chars_result IO_PUBLIC_SYMBOL to_chars(char* first, char* last, bool value, str_bool_format fmt) noexcept
{
    to_chars_result ret = {nullptr, std::errc()};
    std::size_t buf_size = memory_traits::distance(first,last);
    if( 0 == buf_size ) {
        ret.ec = std::errc::no_buffer_space;
    }
    else {
        switch(fmt) {
        case str_bool_format::true_false:
            ret.ptr = detail::fmt_true_false(first,buf_size,value);
            break;
        case str_bool_format::yes_no:
            ret.ptr = detail::fmt_yes_no(first,buf_size,value);
            break;
        }
        if(nullptr == ret.ptr)
            ret.ec = std::errc::no_buffer_space;
    }
    return ret;
}

from_chars_result IO_PUBLIC_SYMBOL from_chars(const char* first,const char* last, bool& value) noexcept
{
    std::size_t buf_size = memory_traits::distance(first,last);
    from_chars_result ret = {nullptr, std::errc()};
    if( 2 > buf_size ) {
        ret.ec = std::errc::invalid_argument;
    }
    else {
        char tmp[8] = { '\0' };
        const char* s = first;
        while( io_isspace(*s) && (s < last+1) ) {
            ++s;
            if(last == s) {
                ret.ec = std::errc::illegal_byte_sequence;
                return ret;
            }
        }
        std::size_t max_len = buf_size > 5 ? 5 : buf_size;
        for(std::size_t i=0; i < max_len; i++) {
            tmp[i] = io_tolower(*s);
            ++s;
        }
        if( 0 == io_memcmp(tmp,"no",2) ) {
            value = false;
            ret.ptr = first + 2;
        }
        else if(0 == io_memcmp(tmp,"yes",3)) {
            value = true;
            ret.ptr = first + 3;
        }
        else if(0 == io_memcmp(tmp,"true",4)) {
            value = true;
            ret.ptr = first + 4;
        }
        else if(0 == io_memcmp(tmp,"false",5) ) {
            value = false;
            ret.ptr = first + 5;
        }
        else {
            ret.ec = std::errc::result_out_of_range;
        }
    }
    return ret;
}

} // namespace io
