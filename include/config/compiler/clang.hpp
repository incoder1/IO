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
#ifndef __IO_CLANG_HPP_INCLUDED__
#define __IO_CLANG_HPP_INCLUDED__

#pragma once

#if __cplusplus < 201103L
#	 error "This library requires at least C++ 11 standard compatiable compiler.\
	 Check your compiller options must have -std=c++[11,14,17,...]"
#endif // CPP11 detection

#if __cplusplus >= 201402L
#	define __HAS_CPP_14 1
#endif // C++ 14

#if __cplusplus >= 201703L
#	define __HAS_CPP_17 1
#endif // C++ 17

#if __cplusplus >= 202002L
#	define __HAS_CPP_20 1
#endif // C++ 20

#include <cstddef>
#include <cstdint>
#include <climits>

#define HAS_PRAGMA_ONCE 1

#if __has_feature(cxx_rtti) && !defined(IO_NO_RTTI)
#	define IO_NO_RTTI 1
#endif

#if __has_feature(cxx_exceptions) && !defined(IO_NO_EXCEPTIONS)
#	define IO_NO_EXCEPTIONS 1
#endif

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#	define IO_IS_LITTLE_ENDIAN  1
#endif // __ORDER_LITTLE_ENDIAN__

#if defined(__LP64__) || defined(_LP64)
#	define IO_CPU_BITS_64 1
#else
#	define IO_CPU_BITS_32 1
#endif // __LP64__

// should not be used unless something specific, i.e. inline assembly based function
#ifndef __forceinline
#	if __has_attribute(always_inline)
#		define __forceinline __attribute__((always_inline))
#	else
#		define __forceinline inline
#	endif // __has_attribute(always_inline)
#endif // __forceinline

#ifndef __noinline
#	if __has_attribute(noinline)
#		define __noinline __attribute__ ((noinline))
#	else
#		define __noinline
#	endif
#endif // IO_NO_INLINE

#ifdef IO_CPU_BITS_64
#	define io_size_t_abs(__x) __builtin_llabs( (__x) )
#else
#	define io_size_t_abs(__x) __builtin_labs( (__x) )
#endif

#if __has_builtin(__builtin_alloca)
#	define io_alloca(__x) __builtin_alloca( (__x) )
#else
#	define io_alloca(__x) _alloca( (__x) )
#endif

#define io_freea(__x) assert(nullptr != (__x))

#if __has_builtin(__builtin_memmove)
#	define io_memmove(__dst, __src, __bytes) __builtin_memmove( (__dst), (__src), (__bytes) )
#else
#	define io_memmove(__dst, __src, __bytes) std::memmove( (__dst), (__src), (__bytes) )
#endif

#if __has_builtin(__builtin_memset)
#	define io_memset(__p,__v,__bytes) __builtin_memset( (__p), (__v), (__bytes) )
#else
#	define io_memset(__p,__v,__bytes) std::memset( (__p), (__v), (__bytes) )
#endif

#if __has_builtin(__builtin_memcmp)
#	define io_memcmp(__p1,__p2,__bytes) __builtin_memcmp((__p1),(__p2),(__bytes))
#else
#	define io_memcmp(__p1,__p2,__bytes) std::memcmp((__p1),(__p2),(__bytes))
#endif

#if __has_builtin(__builtin_bzero)
#	define io_zerro_mem(__p,__bytes) __builtin_bzero( (__p), (__bytes) )
#else
#	define io_zerro_mem(__p,__bytes) std::memset( (__p), 0, (__bytes) )
#endif

#if __has_builtin(__builtin_memchr)
#	define io_memchr(__s,__c,__n) __builtin_memchr((__s),(__c),(__n))
#else
#	define io_memchr(__s,__c,__n) std::memchr((__s),(__c),(__n))
#endif

#if __has_builtin(__builtin_strstr)
#	define io_strstr(__s1,__s2) __builtin_strstr( (__s1), (__s2) )
#else
#	define io_strstr(__s1,__s2) std::strstr( (__s1), (__s2) )
#endif

#if __has_builtin(__builtin_strchr)
#	define io_strchr(__s,__c) __builtin_strchr( (__s), (__c) )
#else
#	define io_strchr(__s,__c) std::strchr( (__s), (__c) )
#endif

#if __has_builtin(__builtin_strlen)
#	define io_strlen(__s) __builtin_strlen( (__s) )
#else
#	define io_strlen(__s) std::strlen( (__s) )
#endif

#if __has_builtin(__builtin_strcpy)
#	define io_strcpy(__s1,__s2) __builtin_strcpy( (__s1), (__s2) )
#else
#	define io_strcpy(__s1,__s2) std::strcpy( (__s1), (__s2) )
#endif

#if __has_builtin(__builtin_strcat)
#	define io_strcat(__dst,__src) __builtin_strcat( (__dst) , (__src) )
#else
#	define io_strcat(__dst,__src) std::strcat( (__dst) , (__src) )
#endif

#if __has_builtin(__builtin_strcmp)
#	define io_strcmp(__lsh,__rhs) __builtin_strcmp( (__lsh), (__rhs) )
#else
#	define io_strcmp(__lsh,__rhs) std::strcmp( (__lsh), (__rhs) )
#endif

#if __has_builtin(__builtin_strncmp)
#	define io_strncmp(__lsh,__rhs,__num) __builtin_strncmp( (__lsh), (__rhs), (__num) )
#else
#	define io_strncmp(__lsh,__rhs,__num) std::strncmp( (__lsh), (__rhs), (__num) )
#endif

#if __has_builtin(__builtin_strspn)
#	define io_strspn(__s, __p) __builtin_strspn( (__s), (__p) )
#else
#	define io_strspn(__s, __p) std::strspn( (__s), (__p) )
#endif

#if __has_builtin(__builtin_strcspn)
#	define io_strcspn(__s, __p) __builtin_strcspn( (__s), (__p) )
#else
#	define io_strcspn(__s, __p) std::strcspn( (__s), (__p) )
#endif

#if __has_builtin(__builtin_strpbrk)
#	define io_strpbrk(__s, __p) __builtin_strpbrk( (__s), (__p) )
#else
#	define io_strpbrk(__s, __p) std::strpbrk( (__s), (__p) )
#endif

#if __has_builtin(__builtin_strchr)
#	define io_strchr(__s,__c) __builtin_strchr( (__s), (__c) )
#else
#	define io_strchr(__s,__c) std::strchr( (__s), (__c) )
#endif

#if __has_builtin(__builtin_isalpha)
#	define io_isalpha(__ch) __builtin_isalpha((__ch))
#else
#	define io_isalpha(__ch) std::isalpha((__ch))
#endif

#if __has_builtin(__builtin_isspace)
#	define io_isspace(__ch) __builtin_isspace((__ch))
#else
#	define io_isspace(__ch) std::isspace((__ch))
#endif

#if __has_builtin(__builtin_islower)
#	define io_islower(__ch) __builtin_islower((__ch))
#else
#	define io_islower(__ch) std::islower((__ch))
#endif

#if __has_builtin(__builtin_isupper)
#	define io_isupper(__ch) __builtin_isupper((__ch))
#else
#	define io_isupper(__ch) std::isupper((__ch))
#endif

#if __has_builtin(__builtin_isdigit)
#	define io_isdigit(__ch) __builtin_isdigit((__ch))
#else
#	define io_isdigit(__ch) std::isdigit((__ch))
#endif

#if __has_builtin(__builtin_tolower)
#	define io_tolower(__ch) __builtin_tolower((__ch))
#else
#	define io_tolower(__ch) std::tolower((__ch))
#endif

#if __has_builtin(__builtin_toupper)
#	define io_toupper(__ch) __builtin_toupper((__ch))
#else
#	define io_toupper(__ch) std::toupper((__ch))
#endif

#if __has_builtin(__builtin_snprintf)
#	define io_snprintf __builtin_snprintf
#else
#	define io_snprintf std::snprintf
#endif

// in case of intel compiler
// in case of gcc or clang
#define io_clz(__x) __builtin_clz((__x))

#ifdef __LP64__
#	define io_size_t_clz(__x) __builtin_clzll((__x))
#else
#	define io_size_t_clz(__x) __builtin_clzl((__x))
#endif // __LP64__

#if __has_builtin(__builtin_bswap16)
#	define io_bswap16(__x) __builtin_bswap16((__x))
#else
__forceinline uint16_t io_bswap16(uint16_t val) noexcept
{
    return (val << 8) | (val >> 8 );
}
#endif


#if __has_builtin(__builtin_bswap32)
#	define io_bswap32(__x) __builtin_bswap32((__x))
#else
inline uint32_t io_bswap32(uint32_t val) noexcept
{
    val = ((val << 8) & 0xFF00FF00 ) | ((val >> 8) & 0xFF00FF );
    return (val << 16) | (val >> 16);
}
#endif

#if __builtin_bswap64(__builtin_bswap64)
#	define io_bswap64(__x) __builtin_bswap64((__x))
#else
inline uint64_t io_bswap64(uint64_t val) noexcept
{
    val = ((val << 8) & 0xFF00FF00FF00FF00ULL ) | ((val >> 8) & 0x00FF00FF00FF00FFULL );
    val = ((val << 16) & 0xFFFF0000FFFF0000ULL ) | ((val >> 16) & 0x0000FFFF0000FFFFULL );
    return (val << 32) | (val >> 32);
}
#endif // __builtin_bswap64

#ifndef IO_PUSH_IGNORE_UNUSED_PARAM

#	define IO_PUSH_IGNORE_UNUSED_PARAM\
			_Pragma("clang diagnostic push")\
			_Pragma("clang diagnostic ignored \"-Wunused-parameter\" ")

#	define IO_POP_IGNORE_UNUSED_PARAM _Pragma("clang diagnostic pop")

#endif // IO_PUSH_IGNORE_UNUSED_PARAM


#define io_likely(__expr__) __builtin_expect(!!(__expr__), 1)
#define io_unlikely(__expr__) __builtin_expect(!!(__expr__), 0)
#define io_unreachable __builtin_unreachable();

// C++ 20 features test
#ifdef __HAS_CPP_20

#ifdef __cpp_char8_t
#  define IO_HAS_CHAR8_T 1
#endif // defined

#ifdef __cpp_concepts
#  define IO_HAS_CONNCEPTS 1
#endif // __cpp_concepts

#endif // __HAS_CPP_20

namespace io {

static __forceinline uint32_t ror32(const uint32_t val,const uint32_t shift) noexcept
{
	return __builtin_rotateright32(val,shift);
}

static __forceinline uint64_t ror64(const uint64_t val,const uint32_t shift) noexcept
{
	return __builtin_rotateright64(val,shift);
}

namespace detail {

// GCC intrinsics for atomic pointer
// used only to save memory since std::atomic_size_t is 8/16 bytes long
// A size_t for reference counter and another std::atomic<size_t>* this
// should not be used with any non dynamically allocated memory,
// otherwise can bring to undefined results
// use std::atomic in normal case, this one is for const_string sso tricks
// to save additional 4/8 bytes of memory
namespace atomic_traits {

	__forceinline std::size_t inc(std::size_t volatile *ptr) noexcept
	{
		return __c11_atomic_fetch_add(ptr, 1, __ATOMIC_RELAXED);
	}

	__forceinline std::size_t dec(std::size_t volatile *ptr) noexcept
	{
		return __c11_atomic_fetch_sub(ptr, 1, __ATOMIC_RELEASE);
	}

} // atomic_traits

} // namespace detail

} // namespace io

#endif // __IO_CLANG_HPP_INCLUDED__
