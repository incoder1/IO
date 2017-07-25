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
#ifndef HMALLOC_HPP_INCLUDED
#define HMALLOC_HPP_INCLUDED

#include <assert.h>
#include <string>

#include <sys/types.h>
#include <unistd.h>

#include <cstdlib>

namespace io {

#define IO_PREVENT_MACRO

struct memory_traits {

	static inline std::size_t page_size()
	{
		return static_cast<std::size_t>( ::sysconf(_SC_PAGESIZE) );
	}

	static inline void* malloc IO_PREVENT_MACRO (std::size_t count) noexcept
	{
		return std::malloc(count);
	}

	static inline void* realloc IO_PREVENT_MACRO (void * const base, std::size_t new_size) noexcept
	{
	   assert(new_size > 0);
       return std::realloc(base, new_size);
	}

	static inline void free IO_PREVENT_MACRO (void * const ptr) noexcept
	{
		std::free(ptr);
	}

	template<typename T>
	static inline std::size_t distance(const T* less_address,const T* lager_address) noexcept
	{
		std::ptrdiff_t diff = lager_address - less_address;
		return diff > 0 ? static_cast<std::size_t>(diff) : 0;
	}

	template<typename T>
	static inline std::size_t raw_distance(const T* less_address,const T* lager_address) noexcept
	{
		return distance<T>(less_address,lager_address) * sizeof(T);
	}

	template<typename T>
	static inline T* calloc_temporary(std::size_t count) noexcept
	{
		return static_cast<T*>( std::calloc(count,sizeof(T)) );
	}

	template<typename T>
	static inline void free_temporary(T* const ptr) noexcept
	{
		std::free(ptr);
	}

};

} // namesapace io


#endif // HMALLOC_HPP_INCLUDED
