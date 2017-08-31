/*
 *
 * Copyright (c) 2016
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

namespace io {

template<typename T>
class scoped_arr {
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

	scoped_arr& operator=(scoped_arr&& rhs) noexcept
	{
		scoped_arr( std::forward<scoped_arr>(rhs) ).swap( *this );
		return *this;
	}

	constexpr scoped_arr(T* const arr, const std::size_t len, release_function rf) noexcept:
		len_(len),
		mem_(arr),
		rf_(rf)
	{}

	explicit scoped_arr() noexcept:
		scoped_arr(nullptr, 0, nullptr)
	{}

	explicit scoped_arr(const std::size_t len) noexcept:
		scoped_arr(
					memory_traits::calloc_temporary<T>(len),
					len,
					[] (T* const px) {
						memory_traits::free_temporary<T>(px);
					})
	{}

	~scoped_arr() noexcept
	{
		if(nullptr != mem_)
			rf_(mem_);
	}

	operator bool() const noexcept {
		return nullptr != mem_;
	}

	inline void clear() noexcept
	{
        std::memset(mem_, 0, len_  * sizeof(T) );
	}

	inline void swap(scoped_arr& other) noexcept
	{
		std::swap(len_, other.len_);
		std::swap(mem_, other.mem_);
		std::swap(rf_, other.rf_);
	}

	operator T*() const noexcept
	{
		return mem_;
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

} // namespace io

#endif // __IO_SCOPED_ARRAY_HPP_INCLUDED__
