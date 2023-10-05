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
#ifndef __IO_H_ALLOCATOR_HPP_INCLUDED__
#define __IO_H_ALLOCATOR_HPP_INCLUDED__

#include <assert.h>

#include <memory>
#include <new>
#include <type_traits>
#include <utility>

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

namespace io {


template<typename T, class __memory_traits>
class heap_allocator_base {
private:

	template< typename _T>
	static constexpr _T* uncast_void(void * const ptr) noexcept {
		return static_cast<_T*>( ptr );
	}

public:

	typedef std::size_t size_type;
	typedef ptrdiff_t difference_type;
	typedef T* pointer;
	typedef const T* const_pointer;
	typedef T&  reference;
	typedef const T& const_reference;
	typedef T value_type;

	constexpr heap_allocator_base() noexcept
	{}

	constexpr pointer address(reference __x) const noexcept
	{
		return std::addressof(__x);
	}

	constexpr const_pointer address(const_reference __x) const noexcept
	{
		return std::addressof(__x);
	}

	pointer allocate(size_type __n, const void* px = nullptr)
	{
		assert( __n > 0 );
		void *ret = nullptr;
		if( __n > 1 ) {
			const std::size_t al = alignof(value_type) - 1;
			const std::size_t bytes = ((__n * sizeof(value_type) ) + al) & ~al;
			ret = __memory_traits::realloc( const_cast<void*>(px), bytes  );
		}
		else {
			ret = __memory_traits::aligned_alloc( sizeof(value_type), alignof(value_type) );
		}
		if( io_unlikely(nullptr == ret) ) {
			if(nullptr != px) {
				__memory_traits::free( const_cast<void*>(px) );
			}
#ifdef IO_NO_EXCEPTIONS
			return nullptr;
#else
			throw std::bad_array_new_length();
#endif
		}
		return uncast_void<value_type>( ret );
	}

	// __p is not permitted to be a null pointer.
	void deallocate(pointer __p, size_type) noexcept
	{
		assert(nullptr != __p);
		__memory_traits::free(__p);
	}

	template<typename _Up, typename... _Args>
	inline void construct(_Up* const __p, _Args&&... __args) noexcept( noexcept( _Up(std::forward<_Args>(__args)...) ) )
	{
		assert(nullptr != __p);
		::new( static_cast<void *>(__p) ) _Up(std::forward<_Args>(__args)...);
	}

	template<typename _Up>
	inline void destroy(_Up* const __p) noexcept( noexcept( __p->~_Up() ) )
	{
		__p->~_Up();
	}

	constexpr size_type max_size() const noexcept
	{
		return SIZE_MAX / sizeof(value_type);
	}

};

} // namespace io

#endif // __IO_H_ALLOCATOR_HPP_INCLUDED__
