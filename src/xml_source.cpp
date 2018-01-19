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

static constexpr unsigned int ASCII_CP_CODE = 20127;
static constexpr unsigned int ISO_LATIN1_CP_CODE = 28591;
static constexpr unsigned int WINDOWS_LATIN1_CP_CODE = 1252;
static constexpr unsigned int UTF8_CP_CODE = 65001;

// source
s_source source::open(std::error_code& ec, const s_read_channel& src, byte_buffer&& rb) noexcept
{
	uint8_t *pos  = const_cast<uint8_t*>( rb.position().get() );
	s_charset_detector chdet = charset_detector::create(ec);
	if(ec)
		return s_source();
	charset_detect_status chdetstat = chdet->detect(ec, pos, rb.size() );
	if( ec )
		return s_source();
	if( !chdetstat && (chdetstat.confidence() < 0.5f) ) {
		ec = make_error_code(converrc::not_supported);
		return s_source();
	}
	charset ch = chdetstat.character_set();
	s_read_channel text_channel;
	switch( static_cast<unsigned int>(ch.code() ) )
	{
		// UTF-8 or latin1
	case ASCII_CP_CODE:
	case ISO_LATIN1_CP_CODE:
	case WINDOWS_LATIN1_CP_CODE:
	case UTF8_CP_CODE:
		text_channel = src;
		break;
		// Create converter
	default:
		byte_buffer new_rb;
		if(utf_16le_bom::is(pos) || utf_16be_bom::is(pos) ) {
			pos += 2;
			new_rb = byte_buffer::allocate( ec, rb.capacity() );
		}
		else if(utf_32be_bom::is(pos) || utf_32le_bom::is(pos) ) {
			pos += 4;
			new_rb = byte_buffer::allocate( ec, rb.capacity() );
		} else {
			new_rb = byte_buffer::allocate( ec, rb.capacity() << 2 );
		}
		if(ec)
			return s_source();
		s_code_cnvtr cnv = code_cnvtr::open(ec,ch,
								code_pages::UTF_8,
								cnvrt_control::failure_on_failing_chars);
		if(ec)
			return s_source();
		cnv->convert(ec, pos, rb.size(), new_rb);
		rb.swap(new_rb);
		text_channel = conv_read_channel::open(ec, src, cnv );
		if(ec)
			return s_source();
	}
	// skip BOM if any
	if( utf8_bom::is(pos) )
		rb.shift( utf8_bom::len() );
	source *sc = nobadalloc<source>::construct(ec, std::move(text_channel), std::move(rb) );
	return !ec ? s_source(sc) : s_source();
}

s_source source::create(std::error_code& ec,s_read_channel&& src) noexcept
{
	byte_buffer buff = byte_buffer::allocate(ec,READ_BUFF_INITIAL_SIZE);
	if(ec)
		return s_source();
	// charge buffer to detect character set
	uint8_t *pos = const_cast<uint8_t*>(buff.position().get());
	size_t read = src->read(ec, pos, buff.capacity() );
	if(ec)
		return s_source();
	buff.move(read);
	buff.flip();
	return open(ec, src, std::move(buff));
}


source::source(s_read_channel&& src, byte_buffer&& rb) noexcept:
	object(),
	char_shift_(1),
	last_( error::ok ),
	pos_(nullptr),
	end_(nullptr),
	row_(1),
	col_(1),
	src_(src),
	// 1page is minimum
	rb_( std::move(rb) )
{
	pos_ = const_cast<char*>(rb_.position().cdata());
	end_ = const_cast<char*>(rb_.last().cdata());
}

source::~source() noexcept
{}

error source::read_more() noexcept
{
    rb_.clear();
	if(  !rb_.empty() &&  rb_.capacity() < READ_BUFF_MAXIMAL_SIZE) {
		if( !rb_.exp_grow() )
			return error::out_of_memory;
	}
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
	error ec = read_more();
	if(ec == error::ok && !rb_.empty()) {
		pos_ = const_cast<char*>(rb_.position().cdata());
		end_ = const_cast<char*>(rb_.last().cdata());
	} else
        pos_ = end_;
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
    char ret = ch;
	if( cheq('\r', ret) ) {
		if( cheq('\n', *(pos_+1) ) ) {
			++pos_;
			++row_;
			col_ = 1;
			return '\n';
		}
		ret = '\n';
	}
	new_line_or_shift_col(ret);
	return ret;
}


char source::next() noexcept
{
	if( end_ == (pos_+1) ) {
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

