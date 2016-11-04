#ifndef __IO_WIN_HMALLOC_HPP_INCLUDED__
#define __IO_WIN_HMALLOC_HPP_INCLUDED__

#include "winconf.hpp"
#include <assert.h>
#include <limits>

#if defined(IO_SHARED_LIB) && defined(NDEBUG) && defined(IO_IS_MINGW)
// TODO: IO string
#else
#	include <cstdlib>
#endif // defined

namespace io {

#if defined(IO_SHARED_LIB) && defined(IO_NO_EXCEPTIONS) && defined(NDEBUG)

namespace {
	static const ::HANDLE __PROCESS_HEAP__ =  ::GetProcessHeap();
}

inline void* h_malloc(std::size_t count) noexcept
{
	return ::HeapAlloc( __PROCESS_HEAP__, 0, count);
}

inline void h_free(void * const ptr) noexcept
{
	::HeapFree( __PROCESS_HEAP__, 0, ptr);
}

#else

__forceinline void* h_malloc(std::size_t count) noexcept
{
	return std::malloc(count);
}


__forceinline void h_free(void * const ptr) noexcept
{
	std::free(ptr);
}

#endif // IO_SHARED_LIB

} // namesapace io

#endif // __IO_WIN_HMALLOC_HPP_INCLUDED__
