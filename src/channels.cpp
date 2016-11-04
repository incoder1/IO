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
#include "stdafx.hpp"
#include "channels.hpp"

namespace io {

// channel
channel::channel() noexcept:
	ref_count_(0)
{}

channel::~channel() noexcept
{}

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

// free functions

static inline void check_err_with_throw_or_terminate(std::error_code& ec)
{
	if( ec ) {
#ifndef IO_NO_EXCEPTIONS
		throw std::system_error(ec);
#else
		std::printf( ec.message().data() );
		std::exit( ec.value() );
#endif // IO_NO_EXCEPTIONS
	}
}


std::size_t IO_PUBLIC_SYMBOL read_some(const read_channel* ch,uint8_t* const buff, std::size_t bytes)
{
	std::error_code ec;
	std::size_t result = ch->read(ec, buff, bytes);
	check_err_with_throw_or_terminate(ec);
	return result;
}

std::size_t IO_PUBLIC_SYMBOL write_some(const write_channel* ch,const uint8_t* buff, std::size_t bytes)
{
	std::error_code ec;
	std::size_t result = ch->write(ec, buff, bytes);
	check_err_with_throw_or_terminate(ec);
	return result;
}


} // namespace io
