#ifndef __IO_OBJECT_HPP_INCLUDED__
#define __IO_OBJECT_HPP_INCLUDED__

#include "config.hpp"

#include <assert.h>
#include <atomic>

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

#if defined(__IO_WINDOWS_BACKEND__) && defined(IO_SHARED_LIB) && defined(IO_NO_EXCEPTIONS)
	void* operator new(std::size_t size) {
		assert(size >= sizeof(object));
		void *res = io::h_malloc(size);
		if(nullptr == res) {
#ifndef IO_NO_EXCEPTIONS
			throw std::bad_alloc();
#else
		std::new_handler new_handler = std::get_new_handler();
		if(nullptr != new_handler) {
			new_handler();
		} else {
			std::unexpected();
		}
#endif // IO_NO_EXCEPTIONS
		}
		return res;
	}

	void operator delete(void* const ptr) noexcept {
		assert(nullptr != ptr);
		io::h_free(ptr);
	}
#endif // defined

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
