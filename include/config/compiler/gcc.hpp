/*
 *
 * Copyright (c) 2016
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#ifndef __COMPILLER_CONFIG_GCC_HPP_INCLUDED__
#define __COMPILLER_CONFIG_GCC_HPP_INCLUDED__

#if __cplusplus < 201103L
#	 error "This library requires at least C++ 11 standard compatiable compiler.\
	 Check your compiller options must have -std=c++[11,14,17,...]"
#endif // CPP11 detection

#if __cplusplus >= 201703L
#	define __HAS_CPP_17 1
#endif // C++ 17

#pragma once

#include <cstddef>
#include <cstdint>
#include <climits>

#define HAS_PRAGMA_ONCE

// Intel compiler specific
#ifdef __ICC
#	include <immintrin.h>
#endif

#ifndef __GXX_RTTI
#	ifndef IO_NO_RTTI
#		define IO_NO_RTTI
#	endif
#endif

#ifndef IO_NO_INLINE
#	define IO_NO_INLINE __attribute__ ((noinline))
#endif // IO_NO_INLINE

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define IO_IS_LITTLE_ENDIAN  1
#endif // __ORDER_LITTLE_ENDIAN__

#ifndef  __EXCEPTIONS
#	define IO_NO_EXCEPTIONS 1
#endif // exception

#if defined(__i386__) || defined(__i486__) || defined(__i586__) || defined(__i686__) || defined(__x86_64__)
#define IO_CPU_INTEL
#endif // INTEL

#if defined(__LP64__) || defined(__x86_64__) || defined(__MINGW64__)
#define IO_CPU_BITS_64 1
#endif // __LP64__

// should not be used unless something specific, i.e. inline assembly based function
#ifndef __forceinline
#	define __forceinline inline __attribute__((__always_inline__))
#endif // __forceinline

#ifdef IO_CPU_BITS_64
#	define io_size_t_abs(__x) __builtin_llabs( (__x) )
#else
#	define io_size_t_abs(__x) __builtin_labs( (__x) )
#endif

// Force compiler intrinsics to avoid standard library calls
// It allows to use inlined assembly operators even when compiler options off intrinsics
// optimizations with debug builds

//#define io_alloca(__x) __builtin_alloca((__x))

#define io_alloca(__x) __builtin_alloca_with_align( (__x), sizeof(size_t) )

#define io_memmove(__dst, __src, __bytes) __builtin_memmove( (__dst), (__src), (__bytes) )

#define io_memset(__p,__v,__bytes) __builtin_memset( (__p), (__v), (__bytes) )

#define io_memcmp(__p1,__p2,__bytes) __builtin_memcmp((__p1),(__p2),(__bytes))

#define io_zerro_mem(__p,__bytes) __builtin_memset( (__p), 0, (__bytes) )

#define io_memchr(__s,__c,__n) __builtin_memchr((__s),(__c),(__n))

#define io_strstr(__s1,__s2) __builtin_strstr( (__s1), (__s2) )

#define io_strchr(__s,__c) __builtin_strchr( (__s), (__c) )

#define io_strlen(__s) __builtin_strlen( (__s) )

#define io_strcpy(__s1,__s2) __builtin_strcpy( (__s1), (__s2) )

#define io_strcat(__dst,__src) __builtin_strcat( (__dst) , (__src) )

#define io_strcmp(__lsh,__rhs) __builtin_strcmp( (__lsh), (__rhs) )

#define io_strncmp(__lsh,__rhs,__num) __builtin_strncmp( (__lsh), (__rhs), (__num) )

#define io_strspn(__s, __p) __builtin_strspn( (__s), (__p) )

#define io_strcspn(__s, __p) __builtin_strcspn( (__s), (__p) )

#define io_strpbrk(__s, __p) __builtin_strpbrk( (__s), (__p) )

#define io_strchr(__s,__c) __builtin_strchr( (__s), (__c) )

#define io_isalpha(__ch) __builtin_isalpha((__ch))
#define io_isspace(__ch) __builtin_isspace((__ch))
#define io_islower(__ch) __builtin_islower((__ch))
#define io_isupper(__ch) __builtin_isupper((__ch))
#define io_isdigit(__ch) __builtin_isdigit((__ch))
#define io_tolower(__ch) __builtin_tolower((__ch))
#define io_toupper(__ch) __builtin_toupper((__ch))

#define io_bswap16(__x) __builtin_bswap16((__x))

// in case of intel compiler
#ifdef __ICC
#	define io_clz(__x) _lzcnt_u32((__x))
#	ifdef  __LP64__
#		define io_size_t_clz(__x) _lzcnt_u64((__x))
#	else
#		define io_size_t_clz(__x) _lzcnt_u32((__x))
#	endif
#else
// in case of gcc or clang
#	define io_clz(__x) __builtin_clz((__x))
#	ifdef __LP64__
#		define io_size_t_clz(__x) __builtin_clzll((__x))
#	else
#		define io_size_t_clz(__x) __builtin_clzl((__x))
#	endif // __LP64__
#endif // clz


#define io_bswap32(__x) __builtin_bswap32((__x))
#define io_bswap64(__x) __builtin_bswap64((__x))

#ifndef IO_PUSH_IGNORE_UNUSED_PARAM

#	define IO_PUSH_IGNORE_UNUSED_PARAM\
			_Pragma("GCC diagnostic push")\
			_Pragma("GCC diagnostic ignored \"-Wunused-parameter\" ")

#	define IO_POP_IGNORE_UNUSED_PARAM _Pragma("GCC diagnostic pop")

#endif // IO_PUSH_IGNORE_UNUSED_PARAM


#define io_likely(__expr__) __builtin_expect(!!(__expr__), 1)
#define io_unlikely(__expr__) __builtin_expect(!!(__expr__), 0)
#define io_unreachable __builtin_unreachable();

namespace io {
namespace detail {

// GCC intrinsics for atomic pointer
// used only to save memory since std::atomic_size_t is 8/16 bytes long
// A size_t for reference counter and another std::atomic<size_t>* this
// should not be used with any non dynamically allocated memory,
// otherwise can bring to undefined results
namespace atomic_traits {
     __forceinline std::size_t inc(std::size_t volatile *ptr) {
        return __atomic_add_fetch(ptr, 1, __ATOMIC_RELAXED);
    }
    __forceinline std::size_t dec(std::size_t volatile *ptr) {
        return __atomic_sub_fetch(ptr, 1, __ATOMIC_RELEASE);
    }
} // atomic_traits

} // namespace detail
} // namespace io

#endif // __COMPILLER_CONFIG_GCC_HPP_INCLUDED__
