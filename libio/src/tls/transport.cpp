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
#include "io/tls/transport.hpp"

namespace io {

namespace net {

namespace tls {

// synch_transport
synch_transport::synch_transport(io::s_read_write_channel&& raw) noexcept:
	transport(),
	raw_( std::forward<io::s_read_write_channel>(raw) ),
	ec_()
{}

ssize_t synch_transport::pull(void* dst,std::size_t len) noexcept
{
	std::size_t ret = raw_->read(ec_, static_cast<uint8_t*>(dst), len);
	if(ec_)
		return -1;
	else
		return static_cast<ssize_t>(ret);
}

ssize_t synch_transport::push(const void* src,std::size_t len) noexcept
{
	std::size_t ret = raw_->write(ec_, static_cast<const uint8_t*>(src), len);
	if(ec_)
		return -1;
	else
		return static_cast<ssize_t>(ret);
}

std::error_code synch_transport::last_error() noexcept
{
	return ec_;
}

} // namespace tls

} //namespace net

} // namespace io
