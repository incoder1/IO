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

} // namespace io
