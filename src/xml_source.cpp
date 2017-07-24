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
#include "xml_source.hpp"
#include "strings.hpp"

namespace io {

namespace xml {

static const char _eof = std::char_traits<char>::eof();


const std::size_t source::READ_BUFF_INITIAL_SIZE = memory_traits::page_size(); // 4k in most cases
const std::size_t source::READ_BUFF_MAXIMAL_SIZE = 0x1000000; // 16 m

// source
s_source source::create(std::error_code& ec, s_read_channel&& src, byte_buffer&& rb,const charset& ch) noexcept
{
	if(ch != code_pages::UTF_8)
		src = conv_read_channel::open(ec, src, ch,
										code_pages::UTF_8,
										cnvrt_control::failure_on_failing_chars);
	source *sc = nobadalloc<source>::construct(ec, std::move(src), std::forward<byte_buffer>(rb) );
	return !ec ? s_source(sc) : s_source();
}

s_source source::create(std::error_code& ec, s_read_channel&& src, const charset& ch) noexcept
{
	byte_buffer buff = byte_buffer::allocate(ec,READ_BUFF_INITIAL_SIZE);
	if(ec)
		return s_source();
	if(buff.capacity() > 0 )
		return create(ec, std::forward<s_read_channel>(src), std::move(buff), ch );
	ec = std::make_error_code(std::errc::not_enough_memory);
	return s_source();
}

source::source(s_read_channel&& src, byte_buffer&& rb) noexcept:
	object(),
	src_(src),
	rb_( std::move(rb) ), // 1k is minimum
	pos_(nullptr),
	end_(nullptr),
	row_(1),
	col_(1),
	char_shift_(1),
	last_( charge() )
{
	// skip BOM's if any
	if( utf8_bom::is(pos_) )
		pos_ += utf8_bom::len();
	else if(utf_16le_bom::is(pos_) || utf_16be_bom::is(pos_) )
		pos_ += utf_16le_bom::len();
	else if(utf_32be_bom::is(pos_) || utf_32le_bom::is(pos_) )
		pos_ += utf_32le_bom::len();
}

source::~source() noexcept
{}

error source::read_more() noexcept
{
	if(  !rb_.empty() &&  rb_.capacity() < READ_BUFF_MAXIMAL_SIZE) {
		rb_.clear();
		if( !rb_.extend( rb_.capacity() << 1 ) )
			return error::out_of_memory;
	} else
		rb_.clear();
	std::error_code ec;
	size_t read = src_->read(ec, const_cast<uint8_t*>(rb_.position().get()), rb_.capacity() );
	if(ec)
		return error::io_error;
	rb_.move(read);
	rb_.flip();
	return error::ok;
}

error source::charge() noexcept
{
	std::error_code err_code;
	error ec = read_more();
	if(!rb_.empty()) {
		pos_ = const_cast<char*>(rb_.position().cdata());
		end_ = const_cast<char*>(rb_.last().cdata());
	} else {
		pos_ = end_;
		return error::ok;
	}
	return ec;
}

// change position ids
inline void source::new_line_or_shift_col(const char ch)
{
	if( cheq(ch,'\n') ) {
		++row_;
		col_ = 1;
	} else
		++col_;
}

// normalize line endings accodring XML spec
inline char source::normalize_lend(char ch)
{
	if( cheq('\r', ch) ) {
		if( cheq('\n', *(pos_+1) ) ) {
			++pos_;
			++row_;
			col_ = 1;
			return '\n';
		}
		ch = '\n';
	}
	new_line_or_shift_col(ch);
	return ch;
}

char source::next() noexcept
{
	if( pos_+2 > end_ ) {
		last_ = charge();
		if( pos_ == end_ || error::ok != last_ )
			return _eof;
	}
	char result = *pos_;
	if( char_shift_ > 1 ) {
		--char_shift_;
		++pos_;
		return result;
	}
	std::uint8_t ch_size = u8_char_size( result );
	switch(ch_size) {
	case 1:
		result = normalize_lend( result );
		break;
	case 5:
		last_ = error::illegal_chars;
		return _eof;
	default:
		++col_;
		char_shift_ = ch_size;
		break;
	}
	++pos_;
	return result;
}

} // namespace xml

} // namesapce io

