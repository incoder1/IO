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

#include "stdafx.hpp"
#include "hmalloc.hpp"
#include "criticalsection.hpp"

#include <atomic>

namespace io {

namespace win {

class heap_allocator {
private:
	constexpr heap_allocator(::HANDLE heap) noexcept:
		heap_(heap)
	{}
	static void do_destoroy() noexcept {
		heap_allocator *tmp = _instance.load(std::memory_order_acquire);
		if(nullptr != tmp) {
			::HANDLE heap = tmp->heap_;
			::HeapFree(heap, 0, tmp);
			::HeapDestroy(heap);
			_instance.store(nullptr, std::memory_order_release);
		}
	}
	static heap_allocator* create_new() noexcept {
		::HANDLE heap = ::HeapCreate(0,0,0);
		if(INVALID_HANDLE_VALUE != heap) {
			void *raw = ::HeapAlloc(heap, HEAP_NO_SERIALIZE, sizeof(heap_allocator) );
			if(NULL != raw)
				return new (raw) heap_allocator(heap);
			else {
				std::new_handler hnd = std::get_new_handler();
				if(nullptr != hnd)
					hnd();
			}
		}
		return nullptr;
	}
public:
	static const heap_allocator* instance() noexcept {
		heap_allocator *ret = _instance.load(std::memory_order_consume);
		if(nullptr == ret) {
			lock_guard lock(_mtx);
			ret = _instance.load(std::memory_order_consume);
			if(nullptr == ret) {
				std::atexit(&heap_allocator::do_destoroy);
				ret = create_new();
				_instance.store(ret, std::memory_order_release);
			}
		}
		return ret;
	}
	__forceinline void* allocate(const std::size_t bytes) const noexcept {
		return ::HeapAlloc(heap_, HEAP_ZERO_MEMORY, bytes);
	}
	__forceinline void release(void* const ptr) const noexcept {
		::HeapFree(heap_, 0, const_cast<LPVOID>(ptr) );
	}
private:
	::HANDLE heap_;
	static critical_section _mtx;
	static std::atomic<heap_allocator*> _instance;
};

critical_section heap_allocator::_mtx;
std::atomic<heap_allocator*> heap_allocator::_instance(nullptr);


#ifdef __GNUG__

void* IO_MALLOC_ATTR private_heap_alloc(std::size_t bytes) noexcept
{
	return heap_allocator::instance()->allocate(bytes);
}

void IO_PUBLIC_SYMBOL private_heap_free(void * const ptr) noexcept
{
	heap_allocator::instance()->release(ptr);
}

#else

IO_PUBLIC_SYMBOL void* private_heap_alloc(std::size_t bytes) noexcept
{
	return heap_allocator::instance()->allocate(bytes);
}

IO_PUBLIC_SYMBOL void private_heap_free(void * const ptr) noexcept
{
	heap_allocator::instance()->release(ptr);
}

#endif


} // namesapce win

} // namespace io
