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

namespace io {

namespace win {

	void* IO_MALLOC_ATTR private_heap_alloc(std::size_t bytes) noexcept;
	void IO_PUBLIC_SYMBOL private_heap_free(void * const ptr) noexcept;

} // namesapce win

#define IO_PREVENT_MACRO

struct memory_traits {

	static inline std::size_t page_size()
	{
		SYSTEM_INFO si;
		GetSystemInfo(&si);
		return static_cast<std::size_t>( si.dwPageSize );
	}

	static inline void* malloc IO_PREVENT_MACRO (std::size_t bytes) noexcept
	{
		return std::malloc(bytes);
	}

	template<typename T>
	static inline T* malloc_array(std::size_t array_size) noexcept
	{
		assert(0 != array_size);
        return static_cast<T*>( std::calloc(array_size, sizeof(T) ) );
	}

	static inline void free IO_PREVENT_MACRO (void * const ptr) noexcept
	{
		std::free(ptr);
	}

	static inline void* realloc IO_PREVENT_MACRO (void * const base, std::size_t new_size) noexcept
	{
	   assert(new_size > 0);
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
		return static_cast<T*>( win::private_heap_alloc( sizeof(T) * count) );
	}

	template<typename T>
	static inline void free_temporary(T* const ptr) noexcept
	{
		return win::private_heap_free( static_cast<void* const>(ptr) );
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

#endif // __IO_WIN_HMALLOC_HPP_INCLUDED__
