/*
 *
 * Copyright (c) 2017
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef HMALLOC_HPP_INCLUDED
#define HMALLOC_HPP_INCLUDED

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include "config/libs/h_allocator.hpp"

#include <sys/types.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>

namespace io {

#define IO_PREVENT_MACRO

struct memory_traits {

	static inline std::size_t page_size()
	{
		return static_cast<std::size_t>( ::sysconf(_SC_PAGESIZE) );
	}

    /// General propose memory allocation
	static inline void* malloc IO_PREVENT_MACRO (std::size_t bytes) noexcept
	{
	    void *ret = nullptr;
#ifdef __GNUG__
        while( __builtin_expect( nullptr == (ret = std::malloc(bytes) ) , false ) )
#else
		while( nullptr == (ret = std::malloc(bytes) ) )
#endif // __GNUG__
        {
            std::new_handler handler = std::get_new_handler();
            if( nullptr == handler )
                break;
            handler();
		}
        return ret;
	}

	/// Continues memory block allocation of specific type
	/// with 0-ro initialization
	template<typename T>
	static inline T* malloc_array(std::size_t array_size) noexcept
	{
		assert(0 != array_size);
		T *ret = nullptr;
#ifdef __GNUG__
        while( __builtin_expect( nullptr == (ret = std::calloc(array_size, sizeof(T) ) ) , false ) )
#else
		while( nullptr == (ret = std::calloc(array_size, sizeof(T) ) ) )
 #endif // __GNUG__
        {
            std::new_handler handler = std::get_new_handler();
            if( nullptr == handler )
                break;
            handler();
		}
        return static_cast<T*>( ret );
	}

	/// General propose memory block release
	/// WARN! do not use for memory allocated by calloc_temporary
	static inline void free IO_PREVENT_MACRO (void * const ptr) noexcept
	{
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


	/// General propose memory block release
	/// WARN! do not use for memory allocated by calloc_temporary
	static inline void free IO_PREVENT_MACRO (void * const ptr) noexcept
	{
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

	template<typename T>
	static inline std::size_t distance(const T* less_address,const T* lager_address) noexcept
	{
		std::ptrdiff_t diff = lager_address - less_address;
		return diff > 0 ? static_cast<std::size_t>(diff) : 0;
	}

	template<typename T>
	static inline std::size_t raw_distance(const T* less_address,const T* lager_address) noexcept
	{
		return distance<T>(less_address,lager_address) * sizeof(T);
	}

	template<typename T>
	static inline T* calloc_temporary(std::size_t count) noexcept
	{
		return malloc_array<T>(count);
	}

	template<typename T>
	static inline void free_temporary(T* const ptr) noexcept
	{
		std::free(ptr);
	}

};

template<typename T>
class h_allocator: public heap_allocator_base <T, memory_traits>
{
public:

	typedef std::size_t size_type;
	typedef ptrdiff_t difference_type;
	typedef T* pointer;
	typedef const T* const_pointer;
	typedef T&  reference;
	typedef const T& const_reference;
	typedef T value_type;

	typedef std::true_type propagate_on_container_move_assignment;

	template<typename T1>
	struct rebind {
		typedef h_allocator<T1> other;
	};

	constexpr h_allocator() noexcept:
		heap_allocator_base<T, memory_traits>()
	{}

	constexpr h_allocator(const h_allocator& other) noexcept:
		heap_allocator_base<T, memory_traits>( other )
	{}

	template<typename _Tp1>
	constexpr h_allocator(const h_allocator<_Tp1>& other) noexcept
	{}

	~h_allocator() noexcept = default;
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


#endif // HMALLOC_HPP_INCLUDED
