/*
 *
 * Copyright (c) 2016-2019
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

// max size for stack mem buffer
// 4k which is page size in most cases
static constexpr std::size_t MAX_CONVB_STACK_SIZE = 4096;

// conv_read_channel

s_read_channel conv_read_channel::open(std::error_code& ec, const s_read_channel& src, const s_code_cnvtr& conv) noexcept
{
	conv_read_channel *ch = io::nobadalloc<conv_read_channel>::construct(ec,
													src, s_code_cnvtr(conv) );
	return !ec ? s_read_channel(ch) : s_read_channel();
}

s_read_channel conv_read_channel::open(
								std::error_code& ec,
		const s_read_channel& src,
		const charset& from,
		const charset& to,
		const cnvrt_control control) noexcept
{
	s_code_cnvtr conv = code_cnvtr::open(ec, from, to, control);
	if(ec)
		return s_read_channel();
	return open(ec, src, std::move(conv) );
}

conv_read_channel::conv_read_channel(const s_read_channel& src,s_code_cnvtr&& conv) noexcept:
	src_(src),
	conv_( std::forward<s_code_cnvtr>(conv) )
{}


std::size_t conv_read_channel::read(std::error_code& ec,uint8_t* const buff, std::size_t bytes) const noexcept
{
	assert( bytes < SIZE_MAX );
	std::size_t length = bytes - (bytes % 2);
	std::size_t rdbuflen = length >> 2;
	uint8_t* rdbuf;
	if ( rdbuflen <= MAX_CONVB_STACK_SIZE) {
		rdbuf = static_cast<uint8_t*>( io_alloca(rdbuflen) );
	} else {
		rdbuf = memory_traits::calloc_temporary<uint8_t>( rdbuflen );
		if(nullptr == rdbuf) {
			ec = std::make_error_code(std::errc::not_enough_memory);
			return 0;
		}
	}
	std::size_t left = length;
	std::size_t read;
	uint8_t** cvt_it = const_cast<uint8_t**>( &buff );
	uint8_t* rb_it[1];
	while( !ec && left != 0 ) {
		read = src_->read(ec, rdbuf, rdbuflen);
		if(0 == read)
			break;
		rb_it[0] = rdbuf;
		while(!ec && read > 0) {
			conv_->convert(ec, rb_it, read, cvt_it, left);
		}
	}
	if( rdbuflen > MAX_CONVB_STACK_SIZE)
		memory_traits::free_temporary( rdbuf );
	return bytes - left;
}

// conv_write_channel
s_write_channel conv_write_channel::open(std::error_code& ec,const s_write_channel& dst,const s_code_cnvtr& conv) noexcept
{
	conv_write_channel *ch = io::nobadalloc<conv_write_channel>::construct(ec, dst, conv );
	return nullptr != ch ? s_write_channel(ch) : s_write_channel();
}

conv_write_channel::conv_write_channel(const s_write_channel& dst,const s_code_cnvtr& conv) noexcept:
	write_channel(),
	dst_( dst ),
	conv_( conv )
{}

conv_write_channel::~conv_write_channel() noexcept
{}

std::size_t conv_write_channel::convert_some(std::error_code& ec, const uint8_t *src, std::size_t &size, uint8_t *dst) const
{
	std::size_t to_convert = (size << 2);
	std::size_t left_after = to_convert;
	uint8_t* uncv[1] = { const_cast<uint8_t*>(src) };
	uint8_t* conv[1] = { dst };
	conv_->convert(ec, uncv, size, conv, left_after);
	return to_convert - left_after;
}

std::size_t conv_write_channel::write(std::error_code& ec, const uint8_t* buff,std::size_t bytes) const noexcept
{
	std::size_t cnvbuflen = bytes << 2;
	uint8_t* cnvbuff;
	if( cnvbuflen <= MAX_CONVB_STACK_SIZE ) {
		cnvbuff = static_cast<uint8_t*>( io_alloca( cnvbuflen ) );
	} else {
	   cnvbuff = memory_traits::calloc_temporary<uint8_t>( cnvbuflen );
	   if(nullptr == cnvbuff) {
			ec = std::make_error_code(std::errc::not_enough_memory);
			return 0;
	   }
	}

    std::size_t unconv_left = bytes;
   	std::size_t to_write = convert_some(ec, buff, unconv_left, cnvbuff);
	std::size_t ret = (bytes - unconv_left);
   	if(ec)
		return 0;

	uint8_t *wpos = cnvbuff;
	std::size_t written;
	while(!ec && to_write > 0) {
		written = dst_->write(ec, wpos, to_write);
		if(ec)
			ret = 0;
		wpos += written;
		to_write -= written;
	}

	if(cnvbuflen > MAX_CONVB_STACK_SIZE)
		memory_traits::free_temporary( cnvbuff );

	return ret;
}

} // namespace io
