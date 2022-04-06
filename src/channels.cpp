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
#include "channels.hpp"

namespace io {

//read_channel
read_channel::read_channel() noexcept:
	object()
{}

//write_channel
write_channel::write_channel() noexcept:
	object()
{}

// read_write_channel
read_write_channel::read_write_channel() noexcept:
	object(),
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

// asynch_completion_routine

void asynch_completion_routine::received(std::error_code& ec, const s_asynch_channel& source,byte_buffer&& data) noexcept
{}

void asynch_completion_routine::sent(std::error_code& ec, const s_asynch_channel& source,byte_buffer&& data) noexcept
{}

// asynch_channel
asynch_channel::asynch_channel(const s_asynch_completion_routine& routine,const asynch_io_context* context) noexcept:
	object(),
	routine_( routine ),
	context_( context )
{}

asynch_channel::~asynch_channel() noexcept
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
		read = src->read(ec, rbuf.begin(), rbuf.len() );
		if(read > 0 && !ec)
			result += transmit_buffer(ec, dst, rbuf.begin(), read);
		else
			break;
	} while( !ec );
	return result;
}


} // namespace io
