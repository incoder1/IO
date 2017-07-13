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

} // namespace io
