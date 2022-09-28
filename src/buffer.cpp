/*
 *
 * Copyright (c) 2016-2022
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#include "stdafx.hpp"
#include "buffer.hpp"
#include <cstddef>
#include <cmath>

namespace io {

namespace detail {

// mem_block
mem_block mem_block::allocate(const std::size_t size) noexcept
{
	uint8_t *ptr = memory_traits::malloc_array<uint8_t>(size);
	return (nullptr != ptr) ? mem_block( ptr ) : mem_block();
}

mem_block mem_block::wrap(const uint8_t* arr,const std::size_t size) noexcept
{
	uint8_t *ptr = memory_traits::malloc_array<uint8_t>(size);
	if(nullptr != ptr) {
		std::memcpy( ptr, arr, size);
	}
	return mem_block( ptr );
}


inline uint8_t* mem_block::reset_ownership() noexcept
{
	uint8_t* ret = px_;
	px_ = nullptr;
	return ret;
}

} // namespace detail

// byte_buffer
byte_buffer::byte_buffer(detail::mem_block&& arr, std::size_t capacity) noexcept:
	arr_( std::forward<detail::mem_block>(arr) ),
	capacity_( capacity ),
	position_(arr_.get()),
	last_(position_+1)
{}

byte_buffer::byte_buffer(byte_buffer&& other) noexcept:
	arr_( std::move(other.arr_) ),
	capacity_(other.capacity_),
	position_(other.position_),
	last_(other.last_)
{
	other.capacity_ = 0;
	other.position_ = nullptr;
	other.last_ = nullptr;
}


void byte_buffer::move(std::size_t offset) noexcept
{
	if(  offset > available() ) {
		position_ = arr_.get() + capacity_-1;
		last_ = arr_.get() + capacity_;
		return;
	}
	position_ += offset;
	last_ = position_ + 1;
}

uint8_t* byte_buffer::new_empty_block(std::size_t size) noexcept
{
	uint8_t* ret;
	if( nullptr == arr_.get() ) {
		ret = memory_traits::malloc_array<uint8_t>( size );
	}
	else {
		// realloc works slowly then malloc in case of new memory block
		ret = static_cast<uint8_t*>( memory_traits::realloc(arr_.get(), size) );
		io_zerro_mem(ret, size);
	}
	if(nullptr != ret) {
		capacity_ = size;
		position_ = ret;
		last_ = position_ + 1;
	}
	return ret;
}

uint8_t* byte_buffer::reallocated_block(std::size_t size) noexcept
{
	std::size_t pos_offset =  memory_traits::distance( arr_.get(), position_ );
	std::size_t last_offset = memory_traits::distance( arr_.get(), last_);
	uint8_t* ret = static_cast<uint8_t*>( memory_traits::realloc(arr_.get(), size) );
	if(nullptr != ret) {
		capacity_ = size;
		position_ = ret + pos_offset;
		last_ =  ret + last_offset;
		const std::size_t tail = size - last_offset;
		if(tail > 0)
			io_zerro_mem(last_, tail );
	}
	return ret;
}


bool byte_buffer::realloc(std::size_t size) noexcept
{
	uint8_t *ret = empty() ? new_empty_block(size) : reallocated_block(size);
	if( nullptr != ret ) {
		arr_.reset_ownership();
		arr_ = detail::mem_block( ret );
	}
	return nullptr != ret ;
}

bool byte_buffer::extend(std::size_t extend_size) noexcept
{
	return realloc( capacity_ + extend_size );
}


bool byte_buffer::exp_grow() noexcept
{
	return realloc( capacity_ << 1 );
}

bool byte_buffer::ln_grow() noexcept
{
	static constexpr unsigned int SIZE_BITS = sizeof(std::size_t) * CHAR_BIT;
	const unsigned int clz = static_cast<unsigned int>( io_size_t_clz(capacity_) );
	const unsigned int cpt_ln2 = SIZE_BITS - (clz + 1);
	constexpr unsigned int mask = 1;
	unsigned int gs = mask << (cpt_ln2 - 1);
	return extend( gs > sizeof(std::size_t) ?  gs : sizeof(std::size_t) );
}

byte_buffer byte_buffer::allocate(std::error_code& ec, std::size_t capacity) noexcept
{
	detail::mem_block block( detail::mem_block::allocate(capacity) );
	if( io_unlikely( nullptr == block.get() ) ) {
		ec = std::make_error_code(std::errc::not_enough_memory);
		return byte_buffer();
	}
	return byte_buffer( std::move(block), capacity );
}


} // namespace io
