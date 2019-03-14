/*
 *
 * Copyright (c) 2016
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

#define _STATIC_CPPLIB
// disable warnings about defining _STATIC_CPPLIB
#define _DISABLE_DEPRECATE_STATIC_CPPLIB  

#include <intrin.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>

// Intel compiler specific
#ifdef __ICL
#	include <immintrin.h>
#endif

#define HAS_PRAGMA_ONCE

#ifndef _CPPRTTI
#	ifndef IO_NO_RTTI
#		define IO_NO_RTTI
#	endif
#endif

#ifndef IO_NO_INLINE
#	define IO_NO_INLINE __declspec(noinline)
#endif // IO_NO_INLINE

#ifndef  _CPPUNWIND
#	define IO_NO_EXCEPTIONS
// use static STL and stdlib C++ when exeptions off
// to avoid std::unxpected, when exeptions off
#	ifdef _HAS_EXCEPTIONS
#		undef _HAS_EXCEPTIONS
#	endif
#	define _HAS_EXCEPTIONS 0
#	define _STATIC_CPPLIB
#endif // exception

// MS VC doesn't generate big endian code
#define IO_IS_LITTLE_ENDIAN 1

#if defined(_M_IX86) || defined(_M_AMD64)
#	define IO_CPU_INTEL 1
#endif

#if defined(_M_X64) || defined(_M_AMD64) || defined(_M_ARM64) || defined(_WIN64)
#	define IO_CPU_BITS_64 1
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

#pragma intrinsic(_alloca,memset)
#define io_alloca(x) _alloca((x))

#define io_memmove(__dst, __src, __bytes) memmove( (__dst), (__src), (__bytes) )

#define io_memchr(__s,__c,__n) memchr( (__s), (__c), (__n) )

#define io_memset(__p,__v,__bytes) memset( (__p), (__v), (__bytes) )

#define io_memcmp(__p1,__p2,__bytes) memcmp( (__p1), (__p2),(__bytes) )

#define io_zerro_mem(__p,__bytes) memset( (__p), 0, (__bytes) )

#define io_strcat(__dst,__src) strcat( (__dst) , (__src) )

#define io_strstr(__s1,__s2) strstr( (__s1), (__s2) )

#define io_strchr(__s,__c) strchr( (__s), (__c) )

#define io_strlen(__s) strlen( (__s) )

#define io_strcmp(__lsh,__rhs) strcmp( (__lsh), (__rhs) )

#define io_strncmp(__lsh,__rhs,__num) strncmp( (__lsh), (__rhs), (__num) )

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
namespace detail {

// MS VC++ atomic Intrinsics
namespace atomic_traits {
#	ifdef _M_X64 // 64 bit instruction set
	__forceinline size_t inc(size_t volatile *ptr) {
		__int64 volatile *p = reinterpret_cast<__int64 volatile*>(ptr);
#	ifdef _M_ARM
		return static_cast<size_t>( _InterlockedIncrement64_nf(p) );
#	else
		// Intel/AMD x64
		return static_cast<size_t>( _InterlockedIncrement64(p) );
#	endif // _M_ARM
	}
	__forceinline size_t dec(size_t volatile *ptr) {
		__int64 volatile *p = reinterpret_cast<__int64 volatile*>(ptr);
		return static_cast<size_t>( _InterlockedDecrement64(p) );
	}

#	else // 32 bit instruction set
	__forceinline size_t inc(size_t volatile *ptr) {
	_long volatile *p = reinterpret_cast<long volatile*>(ptr);
#	ifdef _M_ARM
		return static_cast<size_t>( _InterlockedIncrement_nf(p) );
#	else // Intel/AMD x32
		return static_cast<size_t>( _InterlockedIncrement(p) );
#	endif // _M_ARM
	}
	__forceinline size_t dec(size_t volatile *ptr) {
		long volatile *p = reinterpret_cast<long volatile*>(ptr);
		return static_cast<size_t>( _InterlockedDecrement(p) );
	}
#	endif // 32 bit instruction set
} // atomic_traits

} // namespace detail
} // namespace io



#endif // __COMPILLER_CONFIG_MSVC_HPP_INCLUDED__