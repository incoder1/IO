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
#	error "This library requires at least C++ 11 standard compiler"
#endif // CPP11 detection


#pragma once

#include <cstddef>
#include <cstdint>

#if __cplusplus < 201103L
#	error "This library requires at least C++ 11 standard compatiable compiler.\
	Check your compiller options must have -std=c++[11,14,17,...]"
#endif // CPP11 detection

#define HAS_PRAGMA_ONCE

#if defined(__MINGW32__) || defined(__MINGW64__)
#	define IO_IS_MINGW
#endif // defined

#ifndef __GXX_RTTI
#	ifndef IO_NO_RTTI
#		define IO_NO_RTTI
#	endif
#endif

#ifndef IO_NO_INLINE
#	define IO_NO_INLINE __attribute__ ((noinline))
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
#	define __forceinline inline __attribute__((__always_inline__))
#endif // __forceinline

#ifdef IO_CPU_BITS_64
#	define io_size_t_abs(__x) __builtin_llabs( (__x) )
#else
#	define io_size_t_abs(__x) __builtin_labs( (__x) )
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

#define io_strpbrk(__s, __p) __builtin_strpbrk( (__s), (__p) )

#define io_strchr(__s,__c) __builtin_strchr( (__s), (__c) )

#define io_isalpha(__ch) __builtin_isalpha((__ch))
#define io_isspace(__ch) __builtin_isspace((__ch))
#define io_islower(__ch) __builtin_islower((__ch))
#define io_isupper(__ch) __builtin_isupper((__ch))
#define io_isdigit(__ch) __builtin_isdigit((__ch))
#define io_tolower(__ch) __builtin_tolower((__ch))
#define io_toupper(__ch) __builtin_toupper((__ch))

#define io_bswap16 __builtin_bswap16
#define io_clz __builtin_clz

#ifndef IO_IS_MINGW

#define io_bswap32 __builtin_bswap32
#define io_bswap64 __builtin_bswap64

// some MinGW/MinGW old compiler version making library calls instead of intrict,
// even when -mmovbe
// So replace the call by inline assembler
#elif defined(IO_CPU_INTEL)

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

#else

// non Intel CPU and MinGW is used ( assume this is not practically possible)

template<typename int16_type>
__forceinline int16_type io_bswap16(int16_type x)
{
	return ( (x << 8) & 0xFF00) ) | ( (x >> 8) & 0x00FF );
}

template<typename int32_type>
__forceinline int32_type io_bswap32(int32_type x)
{
	return ((x << 24) & 0xff000000 ) |
		   ((x <<  8) & 0x00ff0000 ) |
		   ((x >>  8) & 0x0000ff00 ) |
		   ((x >> 24) & 0x000000ff );
}

template<typename int64_type>
__forceinline int64_type io_bswap64(int64_type x) {
#ifdef _LP64
	/*
	 * Assume we have wide enough registers to do it without touching
	 * memory.
	 */
	return  ( (x << 56) & 0xff00000000000000UL ) |
		( (x << 40) & 0x00ff000000000000UL ) |
		( (x << 24) & 0x0000ff0000000000UL ) |
		( (x <<  8) & 0x000000ff00000000UL ) |
		( (x >>  8) & 0x00000000ff000000UL ) |
		( (x >> 24) & 0x0000000000ff0000UL ) |
		( (x >> 40) & 0x000000000000ff00UL ) |
		( (x >> 56) & 0x00000000000000ffUL );
#else
	/*
	 * Split the operation in two 32bit steps.
	 */
	uint32_t tl, th;
	th = io_bswap32((uint32_t)(x & 0x00000000ffffffffULL));
	tl = io_bswap32((uint32_t)((x >> 32) & 0x00000000ffffffffULL));
	return ((int64_type)th << 32) | tl;
#endif
}

#endif // IO_IS_MINGW


#ifndef IO_PUSH_IGNORE_UNUSED_PARAM

#	define IO_PUSH_IGNORE_UNUSED_PARAM\
			_Pragma("GCC diagnostic push")\
			_Pragma("GCC diagnostic ignored \"-Wunused-parameter\" ")

#	define IO_POP_IGNORE_UNUSED_PARAM _Pragma("GCC diagnostic pop")

#endif // __GNUC__


#endif // __COMPILLER_CONFIG_GCC_HPP_INCLUDED__
