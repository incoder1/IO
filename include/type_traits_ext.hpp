#ifndef __IO_TYPE_TRAITS_EXT_HPP_INCLUDED__
#define __IO_TYPE_TRAITS_EXT_HPP_INCLUDED__

#include "config.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <type_traits>

namespace io {

template<bool>
struct __conditional {
    template<typename _Tp, typename>
    using type = _Tp;
};

template<>
struct __conditional<false> {
    template<typename, typename _Up>
    using type = _Up;
};

template<bool _Cond, typename _If, typename _Else>
using __conditional_t
    = typename __conditional<_Cond>::template type<_If, _Else>;

template<typename...>
struct __or_;

template<>
struct __or_<>
    : public std::false_type {
};

template<typename _B1>
struct __or_<_B1>
    : public _B1 {
};

template<typename _B1, typename _B2>
struct __or_<_B1, _B2>
    : public __conditional_t<_B1::value, _B1, _B2> {
};

template<typename _B1, typename _B2, typename _B3, typename... _Bn>
struct __or_<_B1, _B2, _B3, _Bn...>
    : public __conditional_t<_B1::value, _B1, __or_<_B2, _B3, _Bn...>> {
};

template<typename _Tp>
using __remove_cv_t = typename std::remove_cv<_Tp>::type;

template<typename _Tp, typename... _Types>
using __is_one_of = __or_<std::is_same<_Tp, _Types>...>;

/// Check if a type is one of the signed integer types.
template<typename _Tp>
using is_signed_integer = __is_one_of<__remove_cv_t<_Tp>,
      signed char, signed short, signed int, signed long, signed long long
      >;

/// Check if a type is one of the unsigned integer types.
template<typename _Tp>
using is_unsigned_integer = __is_one_of<__remove_cv_t<_Tp>,
      unsigned char, unsigned short, unsigned int, unsigned long, unsigned long long
      >;

/// Check if a type is one of the character types
#ifdef IO_HAS_CHAR8_T
template<typename _Tp>
using is_charater = __is_one_of<__remove_cv_t<_Tp>,
      char, wchar_t, char16_t, char32_t, char8_t
      >;
#else
template<typename _Tp>
using is_charater = __is_one_of<__remove_cv_t<_Tp>,
      char, wchar_t, char16_t, char32_t
      >;
#endif // IO_HAS_CHAR8_T


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
