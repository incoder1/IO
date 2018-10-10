/*
 *
 * Copyright (c) 2016-2017
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
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

namespace {

class no_exept_mode {
	no_exept_mode() = delete;
	~no_exept_mode() = delete;
	no_exept_mode(const no_exept_mode&) = delete;
	no_exept_mode operator=(const no_exept_mode&) = delete;
public:
#ifdef IO_NO_EXCEPTIONS
	static constexpr bool is_nothrow = true;
#else
	static constexpr bool is_nothrow = false;
#endif // IO_NO_EXCEPTIONS
};


} // namesapace



template<typename T, class __memory_traits>
class heap_allocator_base {
private:

	template< typename _T>
	static constexpr _T* uncast_void(void * const ptr) noexcept {
#ifdef __GNUG__
		return static_cast<_T*>( __builtin_assume_aligned(ptr, sizeof(_T) ) );
#else
		return static_cast<_T*>( ptr );
#endif
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

	pointer allocate(size_type __n, const void* px = nullptr) noexcept(no_exept_mode::is_nothrow)
	{
		assert( 0 != __n );
		if( io_unlikely( __n > 1) ) {
            void *ret = nullptr;
		    if( io_likely(nullptr == px) )
                ret = __memory_traits::malloc( __n * sizeof(value_type) );
		    else {
		 	   ret = __memory_traits::realloc( const_cast<void*>(px),  __n * sizeof(value_type) );
				if( io_unlikely(nullptr == ret) )
#ifndef IO_NO_EXCEPTIONS
					throw std::bad_array_new_length();
#else
					return nullptr;
#endif // IO_NO_EXCEPTIONS
		    }
            return uncast_void<value_type>(ret);
		}
        return uncast_void<value_type>( __memory_traits::malloc( sizeof(value_type) ) );
	}

	// __p is not permitted to be a null pointer.
	void deallocate(pointer __p, size_type) noexcept
	{
		assert(nullptr != __p);
		__memory_traits::free(__p);
	}

	template<typename _Up, typename... _Args>
	__forceinline void construct(_Up* const __p, _Args&&... __args) noexcept( noexcept( _Up(std::forward<_Args>(__args)...) ) )
	{
		assert(nullptr != __p);
		::new( static_cast<void *>(__p) ) _Up(std::forward<_Args>(__args)...);
	}

	template<typename _Up>
	__forceinline void  destroy(_Up* const __p) noexcept( noexcept( __p->~_Up() ) )
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
