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

#if __cplusplus < 201103L
#	error "This library requires at least C++ 11 standard compatiable compiler. Check compiller options"
#endif // CPP11 detection

#define HAS_PRAGMA_ONCE

#ifndef _CPPRTTI
#	ifndef IO_NO_RTTI
#		define IO_NO_RTTI
#	endif
#endif

#ifndef  _CPPUNWIND
#	define IO_NO_EXCEPTIONS
#endif // exception

#if defined(_M_IX86) || defined(_M_AMD64)
#	define IO_CPU_INTEL
#endif

#if defined(_M_X64) || defined(_M_AMD64) || defined(_M_ARM64) || defined(_WIN64)
#	define IO_CPU_BITS_64
#endif

#ifdef IO_CPU_BITS_64
#define io_size_t_abs(__x) llabs( (__x) )
#else
#define io_size_t_abs(__x) labs( (__x) )
#endif


#define io_bswap32 _byteswap_ulong
#define io_bswap64 _byteswap_uint64

#define io_alloca(x) _alloca((x))

#define io_memmove(__dst, __src, __bytes) memmove( (__dst), (__src), (__bytes) )

#define io_memchr(__s,__c,__n) memchr( (__s), (__c), (__n) )

#define io_memset(__p,__v,__bytes) memset( (__p), (__v), (__bytes) )

#define io_zerro_mem(__p,__bytes) memset( (__p), 0, (__bytes) )

#define io_strstr(__s1,__s2) strstr( (__s1), (__s2) )

#define io_strchr(__s,__c) strchr( (__s), (__c) )

#define io_strlen(__s) strlen( (__s) )

#define io_strcmp(__lsh,__rsh) strcmp( (__lsh), (__rhs) )

#define io_strcpy(__s1,__s2 ) strcpy( (__s1), (__s2) )

#define io_strcspn(__s, __p) strcpy( (__s), (__p) )

#ifndef _DEBUG
#define NDEBUG
#endif

#ifndef IO_PUSH_IGNORE_UNUSED_PARAM

#	define IO_PUSH_IGNORE_UNUSED_PARAM\
			_Pragma("warning( push )")\
			_Pragma("warning( disable : CA1801)")

#	define IO_POP_IGNORE_UNUSED_PARAM _Pragma("warning( pop )")

#endif // ignore unused parameters warning, for stub templates and plased new operators

#endif // __COMPILLER_CONFIG_MSVC_HPP_INCLUDED__