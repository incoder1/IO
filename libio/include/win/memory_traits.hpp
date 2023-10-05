/*
 *
 * Copyright (c) 2016-2020
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef __IO_WIN_HMALLOC_HPP_INCLUDED__
#define __IO_WIN_HMALLOC_HPP_INCLUDED__

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include "config/libs/h_allocator.hpp"

#include <assert.h>
#include <limits>
#include <memory>
#include <cstdlib>

#define IO_PREVENT_MACRO

namespace io {

namespace win {

#ifdef IO_DELCSPEC
IO_PUBLIC_SYMBOL void*
#else
void* IO_MALLOC_ATTR
#endif // IO_DELCSPEC
private_heap_alloc(std::size_t bytes) noexcept;

#ifdef IO_DELCSPEC
IO_PUBLIC_SYMBOL void*
#else
void* IO_MALLOC_ATTR
#endif // IO_DELCSPEC
private_heap_realoc(void* const ptr, const std::size_t new_size) noexcept;

#ifdef IO_DELCSPEC
IO_PUBLIC_SYMBOL void
#else
void IO_PUBLIC_SYMBOL
#endif // IO_DELCSPEC
private_heap_free(void * const ptr) noexcept;

#ifdef IO_DELCSPEC
IO_PUBLIC_SYMBOL std::size_t
#else
std::size_t IO_PUBLIC_SYMBOL
#endif // IO_DELCSPEC
page_size() noexcept;

constexpr std::size_t allign_up(const std::size_t size,const std::size_t alignment) noexcept
{
	return (size + (alignment-1)) & ~(alignment-1);
}

struct memory_traits {

	static inline void* malloc IO_PREVENT_MACRO (std::size_t bytes) noexcept
	{
		return private_heap_alloc(bytes);
	}

	static void* realloc IO_PREVENT_MACRO (void * const base, std::size_t new_size) noexcept
	{
		return private_heap_realoc(base, new_size);
	}

	static inline void* IO_PREVENT_MACRO aligned_alloc(const std::size_t bytes,const std::size_t alignment) noexcept
	{
		return  private_heap_alloc( allign_up(bytes, alignment) );
	}

	static void free IO_PREVENT_MACRO (void * const px) noexcept
	{
		private_heap_free(px);
	}

};

} // namesapce win

/// memory functions traits concept
struct IO_PUBLIC_SYMBOL memory_traits {

	/// returns OS page size
	static std::size_t page_size() noexcept
	{
		return win::page_size();
	}

	/// General propose memory allocation
	static inline void* malloc IO_PREVENT_MACRO (std::size_t bytes) noexcept
	{
		void *ret = nullptr;
		while( io_unlikely( nullptr == (ret = std::malloc(bytes) ) ) ) {
			std::new_handler handler = std::get_new_handler();
			if( nullptr == handler )
				break;
			handler();
		}
		return ret;
	}

	/// Aligned memory allocation
	static inline void* IO_PREVENT_MACRO aligned_alloc(const std::size_t bytes,const std::size_t alignment) noexcept
	{
		return malloc( win::allign_up(bytes,alignment) );
	}

	/// Continues memory block allocation of specific type
	/// with 0-ro initialization
	template<typename T>
	static inline T* malloc_array(std::size_t array_size) noexcept
	{
		assert(0 != array_size);
		void *ret = nullptr;
		while( io_unlikely(nullptr == (ret = std::calloc(array_size,sizeof(T)) ) ) ) {
			std::new_handler handler = std::get_new_handler();
			if( nullptr == handler )
				break;
			handler();
		}
		return static_cast<T*>( ret );
	}

	/// General propose memory block release
	static inline void free IO_PREVENT_MACRO (void * const ptr) noexcept
	{
		assert(nullptr != ptr);
		// replace this one to use jemalloc/tcmalloc etc
		std::free(ptr);
	}

	/// Memory block re-allocation
	static inline void* realloc IO_PREVENT_MACRO (void * const base, std::size_t new_size) noexcept
	{
		assert(new_size > 0);
		// replace this one to use jemalloc/tcmalloc etc
		return std::realloc(base, new_size);
	}

	/// Distance between two pointers as unsigned integral type
	template<typename T>
	static inline std::size_t distance(const T* less_address,const T* lager_address) noexcept
	{
		std::ptrdiff_t diff = lager_address - less_address;
		return diff > 0 ? static_cast<std::size_t>(diff) : 0;
	}

	/// Distance in bytes between two pointers in bytes as unsigned integral type
	template<typename T>
	static inline std::size_t raw_distance(const T* less_address,const T* lager_address) noexcept
	{
		return distance<T>(less_address,lager_address) * sizeof(T);
	}

};

/// STL compatiable allocator which uses memory_traits concept
template<typename T>
class h_allocator: public heap_allocator_base <T, win::memory_traits> {
public:

	typedef std::size_t size_type;
	typedef ptrdiff_t difference_type;
	typedef T* pointer;
	typedef const T* const_pointer;
	typedef T&  reference;
	typedef const T& const_reference;
	typedef T value_type;

	typedef std::true_type propagate_on_container_move_assignment;

	typedef std::true_type is_always_equal;

	template<typename T1>
	struct rebind {
		typedef h_allocator<T1> other;
	};

	constexpr h_allocator() noexcept:
		heap_allocator_base<T, win::memory_traits>()
	{}

	constexpr h_allocator(const h_allocator& other) noexcept:
		heap_allocator_base<T, win::memory_traits>( other )
	{}

	template<typename _Tp1>
	constexpr h_allocator(const h_allocator<_Tp1>&) noexcept
	{}

};


template<typename _Tp>
constexpr inline bool operator==(const h_allocator<_Tp>&, const h_allocator<_Tp>&)
{
	return true;
}

template<typename _Tp>
constexpr inline bool operator!=(const h_allocator<_Tp>&, const h_allocator<_Tp>&)
{
	return false;
}


} // namesapace io

#endif // __IO_WIN_HMALLOC_HPP_INCLUDED__
