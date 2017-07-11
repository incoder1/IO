#ifndef __IO_WIN_HMALLOC_HPP_INCLUDED__
#define __IO_WIN_HMALLOC_HPP_INCLUDED__

#include "winconf.hpp"
#include <assert.h>
#include <limits>
#include <memory>
#include <cstdlib>

namespace io {

#define IO_PREVENT_MACRO

struct memory_traits
{

static inline void* malloc IO_PREVENT_MACRO (std::size_t count) noexcept
{
	return std::malloc(count);
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
static inline T* calloc_temporary(std::size_t count) noexcept {
	return std::get_temporary_buffer<T>(count).first;
}

template<typename T>
static inline void free_temporary(T* const ptr) noexcept {
	std::return_temporary_buffer<T>(ptr);
}

};

namespace detail {

template<typename T>
class scoped_arr {
private:
	static void dispoce(T* const px) noexcept
	{
		memory_traits::free_temporary<T>(px);
	}
public:
	typedef void (*release_function)(T* const);
	scoped_arr(const scoped_arr&)  = delete;
	scoped_arr& operator=(const scoped_arr&) = delete;
	scoped_arr(scoped_arr&& other) noexcept:
		len_( other.len_ ),
		mem_( other.mem_ ),
		rf_( other.rf_)
	{
		other.len_ = 0;
		other.mem_ = nullptr;
		other.rf_ = nullptr;
	}
	scoped_arr operator=(scoped_arr&& rhs) noexcept
	{
		scoped_arr( std::forward<scoped_arr>(rhs) ).swap( *this );
		return *this;
	}
	constexpr scoped_arr(std::size_t len, T* arr, release_function rf) noexcept:
		len_(len),
		mem_(arr),
		rf_(rf)
	{}
	explicit scoped_arr(std::size_t len) noexcept:
		scoped_arr(len, memory_traits::calloc_temporary<T>(len), &scoped_arr::dispoce)
	{}
	inline void swap(scoped_arr& other) noexcept
	{
		std::swap(len_, other.len_);
		std::swap(mem_, other.mem_);
		std::swap(rf_, other.rf_);
	}
	~scoped_arr() noexcept
	{
		if(nullptr != mem_)
			rf_(mem_);
	}
	inline T* get() const noexcept
	{
		return mem_;
	}
	inline std::size_t len() const noexcept
	{
		return len_;
	}
private:
	std::size_t len_;
	T* mem_;
	release_function rf_;
};

} // namespace detail

} // namesapace io

#endif // __IO_WIN_HMALLOC_HPP_INCLUDED__
