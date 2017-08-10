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

#include "../stdafx.hpp"
#include "winconf.hpp"
#include "criticalsection.hpp"
#include <atomic>

namespace io {

namespace win {

class heap_allocator {
private:
	constexpr heap_allocator(::HANDLE heap) noexcept:
		heap_(heap)
	{}
	static void do_destoroy() noexcept
	{
		heap_allocator *tmp = _instance.load(std::memory_order_acquire);
		::HANDLE heap = tmp->heap_;
		::HeapFree(heap, 0, tmp);
		::HeapDestroy(heap);
		_instance.store(nullptr, std::memory_order_release);
	}
public:
	static const heap_allocator* instance() noexcept
	{
		heap_allocator *ret = _instance.load(std::memory_order_relaxed);
		if(nullptr == ret) {
			lock_guard lock(_mtx);
			ret = _instance.load(std::memory_order_acquire);
			if(nullptr == ret) {
				std::atexit(&heap_allocator::do_destoroy);
				::HANDLE heap = ::HeapCreate(0,0,0);
				if(INVALID_HANDLE_VALUE == heap)
					std::terminate();
				void *raw = ::HeapAlloc(heap, HEAP_NO_SERIALIZE, sizeof(heap_allocator) );
				if(NULL == raw)
					std::terminate();
				ret = new (raw) heap_allocator(heap);
				_instance.store(ret, std::memory_order_release);
			}
		}
		return ret;
	}
	inline void* allocate(std::size_t bytes) const noexcept
	{
		return ::HeapAlloc(heap_, HEAP_ZERO_MEMORY, bytes);
	}
	inline void release(void* const ptr) const noexcept
	{
		::HeapFree(heap_, 0, const_cast<LPVOID>(ptr) );
	}
private:
	::HANDLE heap_;
	static critical_section _mtx;
	static std::atomic<heap_allocator*> _instance;
};

critical_section heap_allocator::_mtx;
std::atomic<heap_allocator*> heap_allocator::_instance(nullptr);


void* IO_MALLOC_ATTR private_heap_alloc(std::size_t bytes) noexcept
{
	return heap_allocator::instance()->allocate(bytes);
}

void IO_PUBLIC_SYMBOL private_heap_free(void * const ptr) noexcept
{
	heap_allocator::instance()->release(ptr);
}


} // namesapce win

} // namespace io
