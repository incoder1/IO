/*
 *
 * Copyright (c) 2016-2023
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef __IO_OBJECT_HPP_INCLUDED__
#define __IO_OBJECT_HPP_INCLUDED__

#include <io/config/libio_config.hpp>

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <assert.h>
#include <atomic>
#include <new>

namespace io {

/// !\brief base class for any pointer on reference which should be used from
/// boost intrusive counter smart pointer. Reference counter implementing atomic
/// increment and decrement.
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

private:
	std::atomic_size_t ref_count_;
	friend void intrusive_ptr_add_ref(object* const obj) noexcept {
		obj->ref_count_.fetch_add(1, std::memory_order_relaxed);
	}
	friend void intrusive_ptr_release(object* const obj) noexcept {
		if(1 == obj->ref_count_.fetch_sub(1, std::memory_order_release) ) {
			std::atomic_thread_fence(  std::memory_order_acquire );
			delete obj;
		}
	}
};

DECLARE_IPTR(object);

} // namespace io


#endif // __IO_OBJECT_HPP_INCLUDED__
