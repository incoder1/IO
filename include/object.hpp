/*
 *
 * Copyright (c) 2016
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef __IO_OBJECT_HPP_INCLUDED__
#define __IO_OBJECT_HPP_INCLUDED__

#include "config.hpp"

#include <assert.h>
#include <atomic>
#include <new>

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

namespace io {

class IO_PUBLIC_SYMBOL object {
	object(const object&) = delete;
	object& operator=(const object&) = delete;
	object(object&&) = delete;
	object& operator=(object&&) = delete;
protected:
	constexpr object() noexcept:
		ref_count_(0)
	{}
public:

	virtual ~object() noexcept = default;


#ifdef IO_SHARED_LIB

#ifdef IO_NO_EXCEPTIONS
	void* operator new(std::size_t size) noexcept
#else
	void* operator new(std::size_t size)
#endif // IO_NO_EXCEPTIONS
	{
		void *result;
		result = memory_traits::malloc( size );
		if(NULL != result)
			return result;
#ifdef __GNUC__
		while ( __builtin_expect( (result = std::malloc(size) ) == nullptr, false) ) {
#else
		while( nullptr == ( result = __memory_traits::malloc(size) ) ) {
#endif // __GNUC__
			std::new_handler handler = std::get_new_handler();
			if (nullptr == handler)
#ifdef IO_NO_EXCEPTIONS
				return nullptr;
#else
				throw std::bad_alloc();
#endif // IO_NO_EXCEPTIONS
			handler();
		}
		return result;
	}

	void operator delete(void* const ptr) noexcept {
		assert(nullptr != ptr);
		memory_traits::free(ptr);
	}
#endif // IO_SHARED_LIB

private:
	std::atomic_size_t ref_count_;
	inline friend void intrusive_ptr_add_ref(object* const obj) noexcept {
    	obj->ref_count_.fetch_add(1, std::memory_order_relaxed);
    }
    inline friend void intrusive_ptr_release(object* const obj) noexcept {
    	if(1 == obj->ref_count_.fetch_sub(1, std::memory_order_acquire) ) {
			std::atomic_thread_fence( std::memory_order_release);
			delete obj;
    	}
    }
};

DECLARE_IPTR(object);

} // namespace io


#endif // __IO_OBJECT_HPP_INCLUDED__
