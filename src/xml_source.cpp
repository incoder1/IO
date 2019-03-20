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


const std::size_t source::READ_BUFF_INITIAL_SIZE = memory_traits::page_size(); // 4k in most cases
const std::size_t source::READ_BUFF_MAXIMAL_SIZE = 0x1000000; // 16 m

static constexpr unsigned int ASCII_CP_CODE = 20127;
static constexpr unsigned int ISO_LATIN1_CP_CODE = 28591;
static constexpr unsigned int WINDOWS_LATIN1_CP_CODE = 1252;
static constexpr unsigned int UTF8_CP_CODE = 65001;

static const char NL = '\n';
static const char CR = '\r';


static bool is_utf16(const uint8_t* bom)
{
	return utf_16le_bom::is(bom) || utf_16be_bom::is(bom);
}

static bool is_utf32(const uint8_t* bom)
{
	return utf_32be_bom::is(bom) || utf_32le_bom::is(bom);
}

static s_read_channel open_convert_channel(std::error_code& ec,io::byte_buffer& rb, const uint8_t* pos, const charset& ch, const s_read_channel &src) noexcept
{
	byte_buffer new_rb;

	if( is_utf16(pos) )
		pos += 2;
	else if( is_utf32(pos) )
		pos += 4;

	new_rb = byte_buffer::allocate( ec, rb.capacity() );
	if(ec)
		return s_read_channel();
	s_code_cnvtr cnv = code_cnvtr::open(
						   ec,
						   ch,
						   code_pages::UTF_8,
						   cnvrt_control::failure_on_failing_chars
					   );
	if(ec)
		return s_read_channel();
	cnv->convert(ec, pos, rb.size(), new_rb);
	rb.swap(new_rb);
	return conv_read_channel::open(ec, src, cnv );
}

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
	static const double CONFIDENT = 0.5F;
	if( !chdetstat && (chdetstat.confidence() < CONFIDENT) ) {
		ec = make_error_code(converrc::not_supported);
		return s_source();
	}
	charset ch = chdetstat.character_set();
	s_read_channel text_channel;
	switch( static_cast<unsigned int>(ch.code() ) ) {
	// UTF-8 or latin1
	case ASCII_CP_CODE:
	case ISO_LATIN1_CP_CODE:
	case WINDOWS_LATIN1_CP_CODE:
	case UTF8_CP_CODE:
		text_channel = src;
		if( utf8_bom::is(pos) )
			rb.shift( utf8_bom::len() );
		break;
	// Create converter
	default:
		text_channel = open_convert_channel(ec, rb, pos, ch, src );
		if(ec)
			return s_source();
	}
	source *sc = nobadalloc<source>::construct(ec, std::move(text_channel), std::move(rb) );
	return (nullptr == sc) ? s_source(): s_source(sc);
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
	if( rb_.capacity() < READ_BUFF_MAXIMAL_SIZE ) {
		if( io_unlikely( !rb_.exp_grow() ) )
			return error::out_of_memory;
	}
	std::error_code ec;
	size_t read = src_->read(ec, const_cast<uint8_t*>(rb_.position().get()), rb_.capacity() );
	if( ec )
		return error::io_error;
	rb_.move(read);
	rb_.flip();
	return error::ok;
}

error source::charge() noexcept
{
	error ec = read_more();
	if( io_likely( ec == error::ok && !rb_.empty()) ) {
		pos_ = const_cast<char*>(rb_.position().cdata());
		end_ = const_cast<char*>(rb_.last().cdata());
	}
	else
		pos_ = end_;
	return ec;
}


// normalize line endings according W3C XML spec
inline char source::normalize_line_endings(const char ch)
{
	switch( ch ) {
	case CR:
		if( io_likely( NL == *pos_ ) ) {
			++pos_;
			++row_;
			col_ = 1;
			return NL;
		}
		++col_;
		break;
	case NL:
		++row_;
		col_ = 1;
		break;
	default:
		++col_;
	}
	return ch;
}


char source::next() noexcept
{
	// charge more data from stream, if needed
	if( io_unlikely( end_ == (pos_+1) ) ) {
		last_ = charge();
		if( pos_ == end_ || error::ok != last_ )
			return char8_traits::to_char_type( char8_traits::eof() );
	}

	char ret = *pos_;
	++pos_;

	// check for a multi-byte tail byte
	if( io_unlikely(utf8::ismbtail(ret) ) )
		return ret;

// Compiler specific optimization

// GCC like
#ifdef __GNUG__

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"

	switch( utf8::char_size( ret ) ) {
#ifdef __ICC // case expect not working for intel
	case 1:
#else
	case __builtin_expect(1,true):
#endif // __ICC
		return normalize_line_endings( ret );
	case 2 ... 4:
		++col_;
		return ret;
	default:
		last_ = error::illegal_chars;
		return char8_traits::to_char_type( char8_traits::eof() );
	}

#pragma GCC diagnostic pop

// MS VС++ Like
#else

	switch( utf8::char_size( ret ) ) {
	case 1:
		return normalize_line_endings( ret );
	case 2:
	case 3:
	case 4:
		++col_;
		return ret;
	default:
		last_ = error::illegal_chars;
		return char8_traits::to_char_type( char8_traits::eof() );
	}

#endif // GCC
}

void source::read_until_char(byte_buffer& to,const char lookup,const char illegal) noexcept
{
	char c;
	const char stops[3] = {lookup, illegal, EOF};
	do {
		c = next();
		if( io_unlikely( !to.put(c) ) ) {
			if( !to.ln_grow() || !to.put(c) ) {
				last_ = error::out_of_memory;
				break;
			}
		}
	}
	while( is_not_one(c, stops, 3) );
	if( lookup != c ) {
		last_ = error::illegal_markup;
		to.clear();
	}
}

} // namespace xml

} // namesapce io

