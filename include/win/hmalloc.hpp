#ifndef __IO_WIN_HMALLOC_HPP_INCLUDED__
#define __IO_WIN_HMALLOC_HPP_INCLUDED__

#include "winconf.hpp"
#include <assert.h>
#include <limits>
#include <cstdlib>

namespace io {

inline void* h_malloc(std::size_t count) noexcept
{
	return std::malloc(count);
}

inline void h_free(void * const ptr) noexcept
{
	std::free(ptr);
}

} // namesapace io

#endif // __IO_WIN_HMALLOC_HPP_INCLUDED__
