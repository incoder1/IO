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

#pragma once

#include <cstddef>
#include <cstdint>

#if __cplusplus < 201103L
#	error "This library requires at least C++ 11 standard compatiable compiler. Check compiller options"
#endif // CPP11 detection

#define HAS_PRAGMA_ONCE

#if defined(__MINGW32__) || defined(__MINGW64__)
#define IO_IS_MINGW
#endif // defined

#ifndef __GXX_RTTI
#	ifndef IO_NO_RTTI
#		define IO_NO_RTTI
#	endif
#endif

#ifndef IO_NO_INLINE
#define IO_NO_INLINE __attribute__ ((noinline))
#endif // IO_NO_INLINE

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define IO_IS_LITTLE_ENDIAN
#endif // __ORDER_LITTLE_ENDIAN__

#ifndef  __EXCEPTIONS
#	define IO_NO_EXCEPTIONS
#endif // exception

#if defined(__i386__) || defined(__i486__) || defined(__i586__) || defined(__i686__) || defined(__x86_64__)
#define IO_CPU_INTEL
#endif // INTEL

#if defined(__LP64__) || defined(__x86_64__) || defined(__MINGW64__)
#define IO_CPU_BITS_64 1
#endif // __LP64__

#ifndef __forceinline
#define __forceinline inline __attribute__((__always_inline__))
#endif // __forceinline

#ifdef IO_CPU_BITS_64
#define io_size_t_abs(__x) __builtin_llabs( (__x) )
#else
#define io_size_t_abs(__x) __builtin_labs( (__x) )
#endif

#define io_alloca(__x) __builtin_alloca((__x))

#define io_memmove(__dst, __src, __bytes) __builtin_memmove( (__dst), (__src), (__bytes) )

#define io_memset(__p,__v,__bytes) __builtin_memset( (__p), (__v), (__bytes) )

#define io_memcmp(__p1,__p2,__bytes) __builtin_memcmp((__p1),(__p2),(__bytes))

#define io_zerro_mem(__p,__bytes) __builtin_memset( (__p), 0, (__bytes) )

#define io_memchr(__s,__c,__n) __builtin_memchr( (__s), (__c), (__n) )

#define io_strstr(__s1,__s2) __builtin_strstr( (__s1), (__s2) )

#define io_strchr(__s,__c) __builtin_strchr( (__s), (__c) )

#define io_strlen(__s) __builtin_strlen( (__s) )

#define io_strcpy(__s1,__s2) __builtin_strcpy( (__s1), (__s2) )

#define io_strcmp(__lsh,__rhs) __builtin_strcmp( (__lsh), (__rhs) )

#define io_strcspn(__s, __p) __builtin_strcspn( (__s), (__p) )

#define io_isspace(__ch) __builtin_isspace((__ch))
#define io_islower(__ch) __builtin_islower((__ch))
#define io_isupper(__ch) __builtin_isupper((__ch))
#define io_isdigit(__ch) __builtin_isdigit((__ch))
#define io_tolower(__ch) __builtin_tolower((__ch))
#define io_toupper(__ch) __builtin_toupper((__ch))
#define io_strchr(__s,__c) __builtin_strchr( (__s), (__c) )

#ifndef IO_IS_MINGW

#define io_bswap32 __builtin_bswap32
#define io_bswap64 __builtin_bswap64

#else

inline uint32_t io_bswap32(uint32_t dword)  {
	__asm__ ("bswapl %0" : "=a"(dword)  : "a"(dword) : );
	return dword;
}

#ifdef  __MINGW64__

inline uint64_t io_bswap64(uint64_t qword) {
	__asm__ ("bswapq %0" : "=a"(qword)  : "a"(qword) : );
	return qword;
}

#else

#define io_bswap64 __builtin_bswap64

#endif // IO_x64

#endif // IO_IS_MINGW


#ifndef IO_PUSH_IGNORE_UNUSED_PARAM

#	define IO_PUSH_IGNORE_UNUSED_PARAM\
			_Pragma("GCC diagnostic push")\
			_Pragma("GCC diagnostic ignored \"-Wunused-parameter\" ")

#	define IO_POP_IGNORE_UNUSED_PARAM _Pragma("GCC diagnostic pop")

#endif // __GNUC__


#endif // __COMPILLER_CONFIG_GCC_HPP_INCLUDED__
