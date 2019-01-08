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

#include <type_traits>
#include <cstring>

namespace io {

template<typename T>
class scoped_arr {
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

	constexpr scoped_arr(T* const arr, const std::size_t len, release_function rf) noexcept:
		len_(len),
		mem_(arr),
		rf_(rf)
	{
	    static_assert( std::is_copy_assignable<T>::value || std::is_move_assignable<T>::value, "T must be copy or move assignable" );
	}

	scoped_arr(const T* arr, const std::size_t len) noexcept:
        len_(len),
        mem_(nullptr),
        rf_(
            [] (T* const px) {
                delete [] px;
            }
        )
	{
	    static_assert( !std::is_void<T>::value && (std::is_copy_assignable<T>::value || std::is_move_assignable<T>::value), "T must be copy or move assignable" );
	    assert(0 != len_ && len_ < SIZE_MAX );
        mem_ = new (std::nothrow) T [ len_ ];
        if(nullptr != mem_)
			std::memcpy( mem_, arr, len_);
	}

	constexpr scoped_arr() noexcept:
		scoped_arr(nullptr, 0, nullptr)
	{
	   static_assert( !std::is_void<T>::value && (std::is_copy_assignable<T>::value || std::is_move_assignable<T>::value), "T must be copy or move assignable" );
	}

	explicit scoped_arr(const std::size_t len) noexcept:
		scoped_arr(
					memory_traits::calloc_temporary<T>(len),
					len,
					[] (T* const px) {
						memory_traits::free_temporary<T>(px);
					})
	{
		static_assert( !std::is_void<T>::value && (std::is_copy_assignable<T>::value || std::is_move_assignable<T>::value), "T must be copy or move assignable" );
	}

    scoped_arr(std::initializer_list<T>&& ilist) noexcept:
        scoped_arr( ilist.size() )
	{
	    static_assert( !std::is_void<T>::value && (std::is_copy_assignable<T>::value || std::is_move_assignable<T>::value), "T must be copy or move assignable" );
	    if(nullptr != mem_)
            std::copy(ilist.begin(), ilist.end() , mem_);
	}

	~scoped_arr() noexcept
	{
		if(nullptr != mem_)
			rf_(mem_);
	}

	explicit operator bool() const noexcept {
		return nullptr != mem_;
	}

	inline void clear() noexcept
	{
        io_zerro_mem(mem_, (len_  * sizeof(T)) );
	}

	inline void swap(scoped_arr& other) noexcept
	{
		std::swap(len_, other.len_);
		std::swap(mem_, other.mem_);
		std::swap(rf_, other.rf_);
	}

    const T& operator[](std::size_t index) const noexcept
	{
	    assert( index < len_ );
		return mem_[index];
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
