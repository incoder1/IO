/*
 *
 * Copyright (c) 2016-2021
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#include "stdafx.hpp"
#include "transport.hpp"

namespace io {

namespace net {

namespace tls {

// synch_transport
synch_transport::synch_transport(io::s_read_write_channel&& raw) noexcept:
    transport(),
    raw_( std::forward<io::s_read_write_channel>(raw) )
{}

ssize_t synch_transport::pull(std::error_code& ec,void* dst,std::size_t len) noexcept
{
    std::size_t ret = raw_->read(ec, static_cast<uint8_t*>(dst), len);
    if(ec)
		return -1;
    else
		return static_cast<ssize_t>(ret);
}

ssize_t synch_transport::push(std::error_code& ec, const void* src,std::size_t len) noexcept
{
    std::size_t ret = raw_->write(ec, static_cast<const uint8_t*>(src), len);
    if(ec)
		return -1;
    else
		return static_cast<ssize_t>(ret);
}

// asynch_transport

asynch_transport::asynch_transport(io::s_asynch_channel&& raw) noexcept:
    transport(),
	raw_( std::forward<io::s_asynch_channel>(raw) )
{}

ssize_t asynch_transport::pull(std::error_code& ec, void* dst,std::size_t len) noexcept
{
	raw_->recaive(ec, len, 0 );
	// TODO: wait for completion and notify
	return -1;
}

ssize_t asynch_transport::push(std::error_code& ec, const void* src,std::size_t len) noexcept
{
	raw_->send(ec, io::byte_buffer::wrap(ec, static_cast<const uint8_t*>(src), len), 0 );
	// TODO: wait for completion and notify
	return -1;
}


} // namespace tls

} //namespace net

} // namespace io
