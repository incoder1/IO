/*
 *
 * Copyright (c) 2016-2019
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#include "stdafx.hpp"
#include "channels.hpp"

namespace io {

//read_channel
read_channel::read_channel() noexcept:
	channel()
{}

//write_channel
write_channel::write_channel() noexcept:
	channel()
{}

// read_write_channel
read_write_channel::read_write_channel() noexcept:
	channel(),
	read_channel(),
	write_channel()
{}

read_write_channel::~read_write_channel() noexcept
{}

// random_access_channel
random_access_channel::random_access_channel() noexcept:
	read_write_channel()
{}

random_access_channel::~random_access_channel() noexcept
{}

// asynch_channel
asynch_channel::asynch_channel() noexcept:
	channel()
{}

asynch_channel::~asynch_channel() noexcept
{}

// asynch_read_channel
asynch_read_channel::asynch_read_channel(const asynch_callback& cb) noexcept:
	asynch_channel(),
	callback_(cb)
{}

asynch_read_channel::~asynch_read_channel() noexcept
{}

void asynch_read_channel::on_read_finished(std::error_code& ec,std::size_t pos, byte_buffer&& buff) const
{
	callback_(ec, pos, std::forward<byte_buffer>(buff) );
}

// asynch_write_channel
asynch_write_channel::asynch_write_channel(const asynch_callback& cb) noexcept:
	asynch_channel(),
	callback_(cb)
{}

asynch_write_channel::~asynch_write_channel() noexcept
{}

void asynch_write_channel::on_write_finished(std::error_code& ec,std::size_t pos, byte_buffer&& buff) const
{
	callback_(ec, pos, std::forward<byte_buffer>(buff) );
}

// asynch_read_write_channel
asynch_read_write_channel::asynch_read_write_channel(const asynch_callback& rc, const asynch_callback& wc) noexcept:
	asynch_channel(),
	asynch_read_channel(rc),
	asynch_write_channel(wc)
{}

asynch_read_write_channel::~asynch_read_write_channel() noexcept
{}

// Free functions

std::size_t IO_PUBLIC_SYMBOL transmit_buffer(std::error_code& ec,
				const s_write_channel& ch,
				const uint8_t* buffer, std::size_t size) noexcept
{
	std::size_t ret = 0;
	if( io_unlikely( !ch || nullptr == buffer || 0 == size) ) {
		ec = std::make_error_code( std::errc::invalid_argument );
	} else {
		const uint8_t *b = static_cast<const uint8_t*>(buffer);
		const uint8_t *e = b + size;
		std::size_t written;
		do {
			written = ch->write(ec, b, memory_traits::distance(b,e) );
			ret += written;
			b += written;
		} while( (b < e) && !ec);
	}
	return ret;
}

std::size_t IO_PUBLIC_SYMBOL transmit(std::error_code& ec,const s_read_channel& src, const s_write_channel& dst, unsigned long buff_size) noexcept
{
	if( io_unlikely(!src || !dst || buff_size < 2 ) )  {
		ec = std::make_error_code( std::errc::invalid_argument );
		return 0;
	}

	static constexpr std::size_t al = (sizeof(std::size_t) * 2) - 1;
	static constexpr std::size_t rm = ~al;

	if(0 == buff_size)
		buff_size = memory_traits::page_size();

	const std::size_t al_bs = (static_cast<std::size_t>(buff_size) + al) & rm;
	scoped_arr<uint8_t> rbuf( al_bs );
	if( !rbuf ) {
		ec = std::make_error_code(std::errc::not_enough_memory);
		return 0;
	}
	std::size_t result = 0;
	std::size_t read = 0;
	do {
		read = src->read(ec, rbuf.get(), rbuf.len() );
		if(read > 0 && !ec)
			result += transmit_buffer(ec, dst, rbuf.get(), read);
		else
			break;
	} while( !ec );
	return result;
}


} // namespace io
