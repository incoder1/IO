#ifndef __IO_TYPE_TRAITS_EXT_HPP_INCLUDED__
#define __IO_TYPE_TRAITS_EXT_HPP_INCLUDED__

#include "config.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <type_traits>

namespace io {

template<typename> struct __is_charater : public std::false_type {};
template<> struct __is_charater<char> : public std::true_type {};
template<> struct __is_charater<wchar_t> : public std::true_type {};
template<> struct __is_charater<char16_t> : public std::true_type {};
template<> struct __is_charater<char32_t> : public std::true_type {};
#ifdef IO_HAS_CHAR8_T
template<> struct __is_charater<char8_t> : public std::true_type {};
#endif

template<typename _Tp>
struct is_charater : public __is_charater<typename std::remove_cv<_Tp>::type>::type {
};

#ifdef _GLIBCXX_TYPE_TRAITS

template<typename _Tp>
struct is_unsigned_integer : public std::__is_unsigned_integer<typename std::remove_cv<_Tp>::type>::type {
};

template<typename _Tp>
struct is_signed_integer : public std::__is_signed_integer<typename std::remove_cv<_Tp>::type>::type {
};

#else

template<typename> struct __is_unsigned_integer : public std::false_type {};
template<> struct __is_unsigned_integer<unsigned char> : public std::true_type {};
template<> struct __is_unsigned_integer<unsigned short> : public std::true_type {};
template<> struct __is_unsigned_integer<unsigned int> : public std::true_type {};
template<> struct __is_unsigned_integer<unsigned long int> : public std::true_type {};
template<> struct __is_unsigned_integer<unsigned long long int> : public std::true_type {};

template<typename> struct __is_signed_integer : public std::false_type {};
template<> struct __is_signed_integer<char> : public std::true_type {};
template<> struct __is_signed_integer<short> : public std::true_type {};
template<> struct __is_signed_integer<int> : public std::true_type {};
template<> struct __is_signed_integer<long int> : public std::true_type {};
template<> struct __is_signed_integer<unsigned long long int> : public std::true_type {};
template<> struct __is_unsigned_integer<std::size_t> : public std::true_type {};

template<typename _Tp>
struct is_unsigned_integer : public __is_unsigned_integer<typename std::remove_cv<_Tp>::type>::type {
};

template<typename _Tp>
struct is_signed_integer : public __is_signed_integer<typename std::remove_cv<_Tp>::type>::type {
};

#endif // _GLIBCXX_TYPE_TRAITS

#ifdef IO_HAS_CONNCEPTS
template <typename _Tp>
concept is_charater_v = is_charater<_Tp>::value;

template<typename _Tp>
concept is_unsigned_integer_v = is_unsigned_integer<_Tp>::value;

template<typename _Tp>
concept is_signed_integer_v = is_signed_integer<_Tp>::value;

#endif // IO_HAS_CONNCEPTS

} // namespace io

#endif // __IO_TYPE_TRAITS_EXT_HPP_INCLUDED__
