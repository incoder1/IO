#ifndef HMALLOC_HPP_INCLUDED
#define HMALLOC_HPP_INCLUDED

#include <cstdlib>

namespace io {

#define IO_PREVENT_MACRO

struct memory_traits
{

static inline void* malloc IO_PREVENT_MACRO (std::size_t count) noexcept
{
	return h_malloc(count);
}

static inline void free IO_PREVENT_MACRO (void * const ptr) noexcept
{
	h_free(ptr);
}

};

} // namesapace io


#endif // HMALLOC_HPP_INCLUDED
