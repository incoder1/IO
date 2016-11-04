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
#include "text.hpp"

namespace io {

// conv_read_channel

s_read_channel IO_PUBLIC_SYMBOL conv_read_channel::open(std::error_code& ec,s_read_channel&& src,code_cnvtr&& conv, charset_conv_rate crate) noexcept
{
	conv_read_channel *ch = io::nobadalloc<conv_read_channel>::construct(ec, std::forward<s_read_channel>(src), std::forward<code_cnvtr>(conv), static_cast<int8_t>(crate) );
	return nullptr != ch ? s_read_channel(ch) : s_read_channel();
}

conv_read_channel::conv_read_channel(s_read_channel&& src,code_cnvtr&& conv,int8_t crate) noexcept:
	src_(std::forward<s_read_channel>(src)),
	conv_(std::forward<code_cnvtr>(conv)),
	crate_(crate) {
}

conv_read_channel::~conv_read_channel() noexcept {
}

std::size_t conv_read_channel::read(std::error_code& ec,uint8_t* const buff, std::size_t bytes) const noexcept {
	assert( 0 == bytes % 2 );
	byte_buffer rb = byte_buffer::allocate(ec, (bytes * crate_) );
	if( ec )
		return 0;
	std::size_t read = src_->read(ec, const_cast<uint8_t*>(rb.position().get()), rb.capacity());
	if(0 == read)
		return 0;
	rb.move(read);
	rb.flip();
	std::size_t aval = bytes;
	std::size_t left = rb.size();
	const uint8_t* pos = rb.position().get();
	conv_.convert(ec, pos, left, buff, aval);
	if(ec)
		return 0;
	return (bytes - aval);
}

// conv_write_channel
s_write_channel conv_write_channel::open(std::error_code& ec,s_write_channel&& dst,code_cnvtr&& conv,charset_conv_rate crate) noexcept
{

	conv_write_channel *ch = io::nobadalloc<conv_write_channel>::construct(ec, std::forward<s_write_channel>(dst), std::forward<code_cnvtr>(conv), static_cast<int8_t>(crate) );
	return nullptr != ch ? s_write_channel(ch) : s_write_channel();
}

conv_write_channel::conv_write_channel(s_write_channel&& dst,code_cnvtr&& conv,int8_t crate) noexcept:
	dst_(std::forward<s_write_channel>(dst)),
	conv_(std::forward<code_cnvtr>(conv)),
	crate_(crate)
{}

conv_write_channel::~conv_write_channel() noexcept
{}

std::size_t conv_write_channel::write(std::error_code& ec, const uint8_t* buff,std::size_t bytes) const noexcept
{
	std::size_t buff_size = bytes * crate_;
	detail::scoped_arr<uint8_t> arr( buff_size );
	conv_.convert(ec, buff, bytes, arr.get(), buff_size);
	if(ec)
		return 0;
	std::size_t converted = (bytes * crate_) - buff_size;
	std::size_t result = 0;
	std::size_t written = 0;
	do {
		written = dst_->write(ec, (arr.get()+written), converted);
		result += written;
		converted -= written;
	} while( written > 0);
	return result;
}

} // namespace io
