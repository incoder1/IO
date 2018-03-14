/*
 *
 * Copyright (c) 2018
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#include "stdafx.hpp"
#include "memory_channel.hpp"

namespace io {

s_read_channel memory_read_channel::open(std::error_code& ec, byte_buffer&& buff) noexcept
{
	memory_read_channel *ret = nobadalloc<memory_read_channel>::construct(ec, std::forward<byte_buffer>(buff) );
	return io_likely(nullptr != ret) ? s_read_channel(ret) : s_read_channel();
}

memory_read_channel::memory_read_channel(byte_buffer&& data) noexcept:
	read_channel(),
	data_( std::forward<byte_buffer>(data) ),
	mtx_()
{}

memory_read_channel::~memory_read_channel()  noexcept
{}

std::size_t memory_read_channel::read(std::error_code& ec,uint8_t* const buff, std::size_t bytes) const noexcept
{
	if( data_.empty() )
		return 0;
	lock_guard lock(mtx_);
	std::size_t available = data_.size();
	std::size_t ret = (available >= bytes) ? bytes : available;
	io_memmove(buff, data_.position().get(), ret);
	data_.shift(ret);
	return ret;
}

// memory_write_channel

s_memory_write_channel memory_write_channel::open(std::error_code& ec, std::size_t initial_size) noexcept
{
	byte_buffer buff = byte_buffer::allocate(ec, initial_size);
	if( io_likely(!ec) ) {
		memory_write_channel *ret = nobadalloc<memory_write_channel>::construct(ec, std::move(buff) );
		return io_likely(nullptr != ret) ? s_memory_write_channel(ret) : s_memory_write_channel();
	}
	return s_memory_write_channel();
}


memory_write_channel::memory_write_channel(byte_buffer&& data) noexcept:
	write_channel(),
	data_( std::forward<byte_buffer>(data) ),
	mtx_()
{}

memory_write_channel::~memory_write_channel() noexcept
{}

std::size_t memory_write_channel::write(std::error_code& ec, const uint8_t* buff,std::size_t size) const noexcept
{
	lock_guard lock(mtx_);
	if( size > data_.available() ) {
		if( !data_.exp_grow() && !data_.extend(size) ) {
			ec = std::make_error_code(std::errc::not_enough_memory);
            return 0;
		}
	}
	return data_.put(buff, size);
}

byte_buffer memory_write_channel::data(std::error_code& ec) const noexcept
{
	lock_guard lock(mtx_);
	data_.flip();
	return byte_buffer::wrap(ec, data_.position().get(), data_.size() );
}


} // namespace io
