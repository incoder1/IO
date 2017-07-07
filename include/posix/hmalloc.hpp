#ifndef HMALLOC_HPP_INCLUDED
#define HMALLOC_HPP_INCLUDED

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


#endif // HMALLOC_HPP_INCLUDED
