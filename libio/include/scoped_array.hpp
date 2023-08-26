/*
 *
 * Copyright (c) 2016-2020
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef __IO_SCOPED_ARRAY_HPP_INCLUDED__
#define __IO_SCOPED_ARRAY_HPP_INCLUDED__

#include "config.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <type_traits>

namespace io {


/// !\brief An movable but not copyable heap array reference wrapper. I.e. immutable vector
/// Unlike stl containers like vector, this is fixed size array but holds memory in heap unlike std::array
#ifdef IO_HAS_CONNCEPTS
template<typename T>
	requires( std::is_copy_assignable_v<T> || std::is_move_assignable_v<T> )
#else
template<
	typename T,
	typename = typename std::enable_if<
			std::is_copy_assignable<T>::value ||
			std::is_move_assignable<T>::value
		>::type
>
#endif // IO_HAS_CONNCEPTS
class scoped_arr {
private:
	typedef scoped_arr<T> self_type;

	static inline void temp_free(T* const px) noexcept {
		memory_traits::free_temporary<T>(px);
	}

public:

	typedef void (*release_function)(T* const );

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

	scoped_arr& operator=(scoped_arr&& rhs) noexcept
	{
		scoped_arr( std::forward<scoped_arr>(rhs) ).swap( *this );
		return *this;
	}

	/// Take existing raw heap array ownership
	/// \param arr plain old pointer on memory block, allocated with new T [len], malloc, calloc etc
	/// \param len arr array length in elements
	/// \param rf release memory callback which should be used for memory deallocation i.e. function which calling delete [] or std::free
	constexpr scoped_arr(T* const arr, const std::size_t len, release_function rf) noexcept:
		len_(len),
		mem_(arr),
		rf_(rf)
	{}

	/// Allocates new memory block in heap and deep copy an array into this block
	/// \param arr a pointer on existing array i.e. stack array for example can not be nullptr
	/// \param len arr array length in elements should be grater then 0
	scoped_arr(const T* arr, const std::size_t len) noexcept:
		len_(len),
		mem_( memory_traits::calloc_temporary<T>(len) ),
		rf_( self_type::temp_free )
	{
		assert(nullptr != mem_ && 0 != len_ && len_ < SIZE_MAX );
		io_memmove(mem_, arr, ( len_ * sizeof(T) ) );
	}

	/// Allocates new memory block in heap and deep copy a continues memory block into it
	/// \param begin copied memory block begin
	/// \param end copied memory block end
	scoped_arr(const T* begin, const T* end) noexcept:
		scoped_arr(begin, memory_traits::distance(begin, end) )
	{}

	/// Construct an empty array reference, i.e. en empty reference to be returned in case of error
	constexpr scoped_arr() noexcept:
		scoped_arr(nullptr, 0, nullptr)
	{}

	/// Allocates new memory block in heap, new memory block is fill with 0 values
	/// \param len array size in elements
	explicit scoped_arr(const std::size_t len) noexcept:
		scoped_arr(
			memory_traits::calloc_temporary<T>(len),
			len,
			scoped_arr::temp_free)
	{}

	/// Releases allocated memory on object deconstruction
	~scoped_arr() noexcept
	{
		if(nullptr != mem_)
			rf_(mem_);
	}

	/// Checks whether reference shows on allocated memory block
	/// \return whether shows on allocated memory block
	explicit operator bool() const noexcept
	{
		return nullptr != mem_;
	}

	/// Fill allocated memory by 0 values
	inline void clear() noexcept
	{
		io_zerro_mem(mem_, (len_  * sizeof(T)) );
	}

	/// Swaps this array with another
	inline void swap(scoped_arr& other) noexcept
	{
		std::swap(len_, other.len_);
		std::swap(mem_, other.mem_);
		std::swap(rf_, other.rf_);
	}

	/// Access to array element by index
	T& operator[](std::size_t index) const noexcept
	{
		assert( index < len_ );
		return mem_[index];
	}

	/// Gives array size in bytes
	/// \return array size in bytes
	inline std::size_t bytes() const noexcept {
		return len_ * sizeof(T);
	}

	/// Gives array size in elements
	/// \return array size in elements
	inline std::size_t len() const noexcept {
		return len_;
	}

	/// Gives raw C pointer on first array element
	inline T* begin() noexcept {
		return mem_;
	}

	/// Gives raw C pointer on element after last array element
	inline T* end() noexcept {
		return mem_ + bytes();
	}

	/// Gives raw C const pointer on first array element
	inline const T* cbegin() const noexcept {
		return mem_;
	}

	/// Gives raw C const pointer on element after last array element
	inline const T* cend() const noexcept {
		return mem_ + bytes();
	}

private:
	std::size_t len_;
	T* mem_;
	release_function rf_;
};


} // namespace io

#endif // __IO_SCOPED_ARRAY_HPP_INCLUDED__
