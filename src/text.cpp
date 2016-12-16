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

s_read_channel IO_PUBLIC_SYMBOL conv_read_channel::open(
								std::error_code& ec,
		const s_read_channel& src,
		const charset& from,
		const charset& to) noexcept
{
	s_code_cnvtr conv = code_cnvtr::open(ec, from, to);
	if(ec)
		return s_read_channel();
	conv_read_channel *ch = io::nobadalloc<conv_read_channel>::construct(ec, src, std::move(conv) );
	return !ec ? s_read_channel(ch) : s_read_channel();
}

conv_read_channel::conv_read_channel(const s_read_channel& src,s_code_cnvtr&& conv) noexcept:
	src_(src),
	conv_( std::forward<s_code_cnvtr>(conv) )
{}

conv_read_channel::~conv_read_channel() noexcept
{
}

std::size_t conv_read_channel::read(std::error_code& ec,uint8_t* const buff, std::size_t bytes) const noexcept
{
	assert( bytes < size_t(-1) );
	// allign on size_t and div on 4
	detail::scoped_arr<uint8_t> arr(conv_->requared_conv_buffer(bytes));
	std::size_t read = src_->read(ec, arr.get(), bytes);
	std::size_t converted = arr.len();
	if(!ec) {
		uint8_t *raw = arr.get();
		uint8_t *conv = const_cast<uint8_t*>(buff);
		conv_->convert(ec, &raw, read, &conv, converted);
	}
	return ec ? 0 : bytes - converted;
}

// conv_write_channel
s_write_channel IO_PUBLIC_SYMBOL new_text_conv(std::error_code& ec,const s_write_channel& dst,const s_code_cnvtr& conv) noexcept
{
	conv_write_channel *ch = io::nobadalloc<conv_write_channel>::construct(ec, dst, conv );
	return nullptr != ch ? s_write_channel(ch) : s_write_channel();
}

conv_write_channel::conv_write_channel(const s_write_channel& dst,const s_code_cnvtr& conv) noexcept:
	dst_(dst),
	conv_(conv)
{}

conv_write_channel::~conv_write_channel() noexcept
{}

std::size_t conv_write_channel::write(std::error_code& ec, const uint8_t* buff,std::size_t bytes) const noexcept
{
	uint8_t** src = const_cast<uint8_t**>(&buff);
	std::size_t left = bytes;
	std::size_t avail = left * 4;
	detail::scoped_arr<uint8_t> tmp(avail);
	uint8_t* conv = tmp.get();
	conv_->convert(ec, src, left, static_cast<uint8_t**>(&conv), avail);
	if(!ec && dst_->write(ec, tmp.get(), tmp.len() - avail) > 0 ) {
		return bytes - left;
	}
	return 0;
}

} // namespace io
