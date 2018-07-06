#ifndef __IO_WIN_HMALLOC_HPP_INCLUDED__
#define __IO_WIN_HMALLOC_HPP_INCLUDED__

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include "config/libs/h_allocator.hpp"

#include <assert.h>
#include <limits>
#include <memory>
#include <cstdlib>

namespace io {

namespace win {

#ifdef __GNUG__
	void* IO_MALLOC_ATTR private_heap_alloc(std::size_t bytes) noexcept;
	void IO_PUBLIC_SYMBOL private_heap_free(void * const ptr) noexcept;
#else
	IO_PUBLIC_SYMBOL void* private_heap_alloc(std::size_t bytes) noexcept;
	IO_PUBLIC_SYMBOL void private_heap_free(void * const ptr) noexcept;
#endif


} // namesapce win

#define IO_PREVENT_MACRO

/// memory functions traits concept
struct memory_traits {

	/// returns OS page size
	static inline std::size_t page_size() noexcept
	{
		static ::DWORD ret = 0;
		if(0 == ret) {
			::SYSTEM_INFO si;
			::GetSystemInfo(&si);
			ret = si.dwPageSize;
		}
		return static_cast<std::size_t>( ret );
	}

	/// General propose memory allocation
	static inline void* malloc IO_PREVENT_MACRO (std::size_t bytes) noexcept
	{
	    void *ret = nullptr;
        while( io_unlikely( nullptr == (ret = std::malloc(bytes) ) ) )
        {
            std::new_handler handler = std::get_new_handler();
            if( nullptr == handler )
                break;
            handler();
		}
        return ret;
	}

	/// Continues memory block allocation of specific type
	/// with 0-ro initialization
	template<typename T>
	static inline T* malloc_array(std::size_t array_size) noexcept
	{
		assert(0 != array_size);
		void *ret = nullptr;
		while( io_unlikely(nullptr == (ret = std::calloc(array_size,sizeof(T)) ) ) )
        {
            std::new_handler handler = std::get_new_handler();
            if( nullptr == handler )
                break;
            handler();
		}
        return static_cast<T*>( ret );
	}

	/// General propose memory block release
	/// WARN! do not use for memory allocated by calloc_temporary
	static inline void free IO_PREVENT_MACRO (void * const ptr) noexcept
	{
		assert(nullptr != ptr);
		// replace this one to use jemalloc/tcmalloc etc
		std::free(ptr);
	}

	/// Memory block re-allocation
	static inline void* realloc IO_PREVENT_MACRO (void * const base, std::size_t new_size) noexcept
	{
	   assert(new_size > 0);
		// replace this one to use jemalloc/tcmalloc etc
       return std::realloc(base, new_size);
	}

	/// Distance between two pointers as unsigned integral type
	template<typename T>
	static inline std::size_t distance(const T* less_address,const T* lager_address) noexcept
	{
		std::ptrdiff_t diff = lager_address - less_address;
		return diff > 0 ? static_cast<std::size_t>(diff) : 0;
	}

	/// Distance between two pointers in bytes as unsigned integral type
	template<typename T>
	static inline std::size_t raw_distance(const T* less_address,const T* lager_address) noexcept
	{
		return distance<T>(less_address,lager_address) * sizeof(T);
	}

	/// Temporary propose memory continues memory block allocation of specific type
	/// with 0-ro initialization
	/// this implementation uses additional local application heap
	/// to avoid process default heap fragmentation because of temporary
	/// memory blocks allocations/deallocations
	/// memory allocated by this function must be freed by free_temporary only
	/// behavior of delete [] or std::free is undefined (sigsev or 0x0....5)
	template<typename T>
	static inline T* calloc_temporary(std::size_t count) noexcept
	{
		void *ret = nullptr;
		while( io_unlikely(nullptr == (ret = static_cast<T*>( win::private_heap_alloc( sizeof(T) * count) ) )) )
        {
            std::new_handler handler = std::get_new_handler();
            if( nullptr == handler )
                break;
            handler();
		}
        return static_cast<T*>( ret );
	}

	/// Temporary propose memory block allocated with calloc_temporary only
	template<typename T>
	static inline void free_temporary(T* const ptr) noexcept
	{
		return win::private_heap_free( static_cast<void* const>(ptr) );
	}

};

/// STL compatiable allocator which uses memory_traits concept
template<typename T>
class h_allocator: public heap_allocator_base <T, memory_traits>
{
public:

	typedef std::size_t size_type;
	typedef ptrdiff_t difference_type;
	typedef T* pointer;
	typedef const T* const_pointer;
	typedef T&  reference;
	typedef const T& const_reference;
	typedef T value_type;

    typedef std::true_type propagate_on_container_move_assignment;

	template<typename T1>
	struct rebind {
		typedef h_allocator<T1> other;
	};

	constexpr h_allocator() noexcept:
		heap_allocator_base<T, memory_traits>()
	{}

	constexpr h_allocator(const h_allocator& other) noexcept:
		heap_allocator_base<T, memory_traits>( other )
	{}

	template<typename _Tp1>
	constexpr h_allocator(const h_allocator<_Tp1>&) noexcept
	{}

};


template<typename _Tp>
constexpr inline bool operator==(const h_allocator<_Tp>&, const h_allocator<_Tp>&)
{
	return true;
}

template<typename _Tp>
constexpr inline bool operator!=(const h_allocator<_Tp>&, const h_allocator<_Tp>&)
{
	return false;
}

} // namesapace io

#endif // __IO_WIN_HMALLOC_HPP_INCLUDED__
