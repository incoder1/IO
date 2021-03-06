/*
 *
 * Copyright (c) 2017
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#include "stdafx.hpp"
#include "asynch_channel.hpp"

namespace io {

namespace win {

asynch_channel::asynch_channel(::HANDLE hres, const asynch_callback& rc, const asynch_callback& wc) noexcept:
	asynch_read_write_channel(rc,wc),
	hch_(hres)
{}

asynch_channel::~asynch_channel() noexcept
{}

void asynch_channel::read(std::size_t bytes, std::size_t pos) const noexcept
{
}

void asynch_channel::write(byte_buffer&& buff, std::size_t pos) const noexcept
{
}

bool asynch_channel::cancel_pending() const noexcept
{
	return TRUE == ::CancelIo( hch_ );
}

bool asynch_channel::cancel_all() const noexcept
{
	return TRUE == ::CancelIoEx( hch_, nullptr);
}



} // namespace io {

} // namespace win {

