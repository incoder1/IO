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
#include "blocking_channel.hpp"

namespace io {

namespace net {

namespace tls {

blocking_channel::blocking_channel(s_session&& s) noexcept:
    read_write_channel(),
    session_( std::forward<s_session>(s) )
{}

blocking_channel::~blocking_channel() noexcept
{}

std::size_t blocking_channel::read(std::error_code& ec,uint8_t* const buff, std::size_t bytes) const noexcept
{
    return session_->read(ec, buff, bytes);
}

std::size_t blocking_channel::write(std::error_code& ec, const uint8_t* buff,std::size_t size) const noexcept
{
    return session_->write(ec, buff, size);
}

} // namespace tls

} //namespace net

} // namespace io
