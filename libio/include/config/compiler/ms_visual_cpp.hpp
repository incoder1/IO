/*
 *
 * Copyright (c) 2016-2022
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#ifndef __COMPILLER_CONFIG_MSVC_HPP_INCLUDED__
#define __COMPILLER_CONFIG_MSVC_HPP_INCLUDED__

#pragma once

#if _MSVC_LANG < 201103L
#	error "This library needs at least MS VC++ 15 with /std:c++latest compiller option"
#endif // CPP11 detection

#if _MSVC_LANG >= 201402L
#	define __HAS_CPP_14 1
#endif // CPP14 detected

#if _MSVC_LANG >= 201703L
#	define __HAS_CPP_17 1
#endif // CPP17 detected

#if _MSVC_LANG >= 202002L
#	define __HAS_CPP_20 1
#	define IO_HAS_CONNCEPTS 1
#endif // CPP20 detected

#ifdef IO_BUILD
#	define _CRT_SECURE_NO_WARNINGS 1
#endif

// C++ 20 features test
#ifdef __HAS_CPP_20

#ifdef __cpp_char8_t
#  define IO_HAS_CHAR8_T 1
#endif // defined

#ifdef __cpp_concepts
#  define IO_HAS_CONNCEPTS 1
#endif // __cpp_concepts

#endif // __HAS_CPP_20

#define _STATIC_CPPLIB
// disable warnings about defining _STATIC_CPPLIB
#define _DISABLE_DEPRECATE_STATIC_CPPLIB

#include <intrin.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <BaseTsd.h>

typedef SSIZE_T ssize_t;

#define HAS_PRAGMA_ONCE

#ifndef _CPPRTTI
#	ifndef IO_NO_RTTI
#		define IO_NO_RTTI
#	endif
#endif

#ifndef _CPPUNWIND
#	define IO_NO_EXCEPTIONS
// use static STL and stdlib C++ when exeptions off
// to avoid std::unxpected, when exeptions off
#	ifdef _HAS_EXCEPTIONS
#		undef _HAS_EXCEPTIONS
#	endif
#	define _HAS_EXCEPTIONS 0
#	define _STATIC_CPPLIB
#endif // exception

#ifndef __noinline
#	define __noinline __declspec(noinline)
#endif // IO_NO_INLINE

// MS VC does not generate big endian code at all
#define IO_IS_LITTLE_ENDIAN 1

#if defined(_M_IX86) || defined(_M_AMD64)
#	define IO_CPU_INTEL 1
#endif

#if defined(_M_X64) || defined(_M_AMD64) || defined(_M_ARM64) || defined(_WIN64)
#	define IO_CPU_BITS_64 1
#else
#	define IO_CPU_BITS_32 1
#endif

#ifdef IO_CPU_BITS_64
#	pragma intrinsic(_abs64)
#	define io_size_t_abs(__x) _abs64( (__x) )
#else
#	pragma intrinsic(_labs)
#	define io_size_t_abs(__x) labs( (__x) )
#endif

#pragma intrinsic(_byteswap_ushort,_byteswap_ulong,_byteswap_uint64)

#define io_bswap16 _byteswap_ushort
#define io_bswap32 _byteswap_ulong
#define io_bswap64 _byteswap_uint64

#pragma intrinsic(memset)

#define io_alloca(__bytes) _malloca((__bytes))

#define io_freea(__ptr) _freea((__ptr))

#define io_memmove(__dst, __src, __bytes) memmove( (__dst), (__src), (__bytes) )

#define io_memchr(__s,__c,__n) memchr( (__s), (__c), (__n) )

#define io_memset(__p,__v,__bytes) memset( (__p), (__v), (__bytes) )

#define io_memcmp(__p1,__p2,__bytes) memcmp( (__p1), (__p2),(__bytes) )

#define io_zerro_mem(__p,__bytes) RtlZeroMemory( (__p), (__bytes) )

#define io_strcat(__dst,__src) strcat( (__dst) , (__src) )

#define io_strstr(__s1,__s2) strstr( (__s1), (__s2) )

#define io_strchr(__s,__c) strchr( (__s), (__c) )

#define io_strlen(__s) strlen( (__s) )

#define io_strcmp(__lsh,__rhs) strcmp( (__lsh), (__rhs) )

#define io_strncmp(__lsh,__rhs,__num) strncmp( (__lsh), (__rhs), (__num) )

#define io_strncasecmp(__lsh,__rhs,__num) _strnicmp( (__lsh), (__rhs), (__num) )

#define io_strcpy(__s1,__s2 ) strcpy( (__s1), (__s2) )

#define io_strspn(__s, __p) strspn( (__s), (__p) )

#define io_strcspn(__s, __p) strcspn( (__s), (__p) )

#define io_strpbrk(__s, __p) strpbrk( (__s), (__p) )

#define io_isalpha(__ch) isalpha((__ch))

#define io_isspace(__ch) isspace((__ch))

#define io_islower(__ch) islower((__ch))

#define io_isupper(__ch) isupper((__ch))

#define io_isdigit(__ch) isdigit((__ch))

#define io_tolower(__ch) tolower((__ch))

#define io_toupper(__ch) toupper((__ch))

#define io_snprintf std::snprintf

#ifndef IO_PUSH_IGNORE_UNUSED_PARAM

#	define IO_PUSH_IGNORE_UNUSED_PARAM\
			_Pragma("warning( push )")\
			_Pragma("warning( push )")\
			_Pragma("warning( disable : CA1801)")

#	define IO_POP_IGNORE_UNUSED_PARAM _Pragma("warning( pop )")

#endif // ignore unused parameters warning, for stub templates and plased new operators

#ifdef __ICL
#	define io_likely(__expr__) __builtin_expect(!!(__expr__), 1)
#	define io_unlikely(__expr__) __builtin_expect(!!(__expr__), 0)
#else 
#	define io_likely(__expr__) !!(__expr__)
#	define io_unlikely(__expr__) !!(__expr__)
#endif

#define io_unreachable __assume(0);


// Intel intrinsic
#ifdef __ICL
#	define io_clz(__x) _lzcnt_u32((__x))
#	ifdef  _M_AMD64
#		define io_size_t_clz(__x) _lzcnt_u64((__x))
#	else
#		define io_size_t_clz(__x) _lzcnt_u32((__x))
#	endif
#elif defined(_M_AMD64) || ( defined(_M_IX86_FP) && (_M_IX86_FP >= 2) )
#	pragma intrinsic(__lzcnt, _bittest)
#	define io_clz(__x) __lzcnt((__x))
#	ifdef IO_CPU_BITS_64
#		pragma intrinsic(__lzcnt64)
#		define io_size_t_clz(__x) __lzcnt64((__x))
#	else
#		define io_size_t_clz(__x) __lzcnt((__x))
#	endif
#else
	#pragma intrinsic(_BitScanReverse)
	__forceinline int io_clz(unsigned long x)  noexcept {
		unsigned long ret = 0;
		_BitScanReverse(&ret, x);
		return  static_cast<int>(ret);
	}
#	ifdef _M_X64
#		pragma intrinsic(_BitScanReverse64)
		__forceinline  int io_size_t_clz(unsigned __int64 x) noexcept {
			unsigned long ret = 0;
			_BitScanReverse64(&ret, x);
			return static_cast<int>(ret);
		}
	#else 
		__forceinline int io_size_t_clz(unsigned long x) noexcept {
			unsigned long ret = 0;
			_BitScanReverse(&ret, x);
			return  static_cast<int>(ret);
		}
#	endif 
#endif

namespace io {

#pragma intrinsic(_rotr,_rotr64)

static __forceinline uint32_t ror32(const uint32_t val,const uint32_t shift) noexcept
{
	return _rotr(val,shift);
}

static __forceinline uint64_t ror64(const uint64_t val,const uint32_t shift) noexcept
{
	return _rotr64(val,shift);
}

namespace detail {
	
#ifdef _M_X64
// 64 bit instruction set
#	ifdef _M_ARM64
#		pragma intrinsic(_InterlockedIncrement64_nf)
#		pragma intrinsic(_InterlockedDecrement64_rel)
#		define io_atomic_inc( __x ) _InterlockedIncrement64_nf( (__x) )
#		define io_atomic_dec( __x ) _InterlockedDecrement64_rel( (__x) )
#	else
#		pragma intrinsic(_InterlockedIncrement64)
#		pragma intrinsic(_InterlockedDecrement64)
#		define io_atomic_inc( __x ) _InterlockedIncrement64( (__x) )
#		define io_atomic_dec( __x ) _InterlockedDecrement64( (__x) )
#	endif
#	define io_mword( __x ) reinterpret_cast<__int64 volatile*>( (__x) )
#else
// 32 bit instruction set
#	ifdef _M_ARM
#		pragma intrinsic(_InterlockedIncrement_nf)
#		pragma intrinsic(_InterlockedDecrement_rel)
#		define io_atomic_inc( __x ) _InterlockedIncrement_nf( (__x) )
#		define io_atomic_dec( __x ) _InterlockedDecrement64_rel( (__x) )
#	else
#		pragma intrinsic(_InterlockedIncrement)
#		pragma intrinsic(_InterlockedDecrement)
#		define io_atomic_inc( __x ) _InterlockedIncrement( (__x) )
#		define io_atomic_dec( __x ) _InterlockedDecrement( (__x) )
#	endif
#	define io_mword( __x ) reinterpret_cast<long volatile*>( (__x) )
#endif

// MS VC++ atomic Intrinsics
namespace atomic_traits {
	
	__forceinline size_t inc(size_t volatile *ptr) noexcept {
		return static_cast<size_t>( io_atomic_inc( io_mword(ptr) ) );
	}
	
	__forceinline size_t dec(size_t volatile *ptr) noexcept {
		return static_cast<size_t>( io_atomic_dec( io_mword(ptr) ) );
	}
	
} // =namespace atomic_traits

#undef io_atomic_inc
#undef io_atomic_dec
#undef io_mword

} // namespace detail
} // namespace io



#endif // __COMPILLER_CONFIG_MSVC_HPP_INCLUDED__