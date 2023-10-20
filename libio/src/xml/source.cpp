/*
 *
 * Copyright (c) 2016-2023
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#include "stdafx.hpp"
#include <io/core/string_algs.hpp>

#include "io/xml/source.hpp"

namespace io {

namespace xml {

const std::size_t source::READ_BUFF_INITIAL_SIZE = memory_traits::page_size(); // 4k in most cases
const std::size_t source::READ_BUFF_MAXIMAL_SIZE = source::READ_BUFF_INITIAL_SIZE * source::READ_BUFF_INITIAL_SIZE; // about 16 mb

static constexpr char NL = '\n';
static constexpr char CR = '\r';

static s_read_channel open_convert_channel(std::error_code& ec,io::byte_buffer& rb, const charset* ch, const s_read_channel &src) noexcept
{
	s_read_channel ret;
	byte_buffer new_rb = byte_buffer::allocate( ec, rb.capacity() );
	if(!ec) {
		auto cvt  = charset_converter::open(ec, ch, code_pages::utf8() );
		if(!ec) {
			uint8_t* pos  = const_cast<uint8_t*>( rb.position().get() );
			cvt->convert(ec, pos, rb.size(), new_rb);
			if(!ec) {
				rb.swap(new_rb);
				ret =  src; //conv_read_channel::open(ec, src, cvt );
			}
		}
	}
	return ret;
}

static bool charset_utf8_compatiable(const charset* ch) noexcept
{
	return code_pages::utf8() == ch || code_pages::ascii() == ch || code_pages::cp1252() == ch || code_pages::iso_8859_1() == ch;
}

// source
s_source source::open(std::error_code& ec, const s_read_channel& src, byte_buffer&& rb) noexcept
{
	s_source ret;
	const charset* ch = nullptr;
	// Try to detect stream code page using Byte Order Mark first
	if( utf8_bom::is(rb.position().get()) ) {
		rb.shift(utf8_bom::len());
		ch = code_pages::utf8();
	}
	else if(utf_16le_bom::is(rb.position().get()) ) {
		rb.shift(utf_16le_bom::len());
		ch = code_pages::utf16le();
	}
	else if(utf_16be_bom::is(rb.position().get())) {
		rb.shift(utf_16be_bom::len());
		ch = code_pages::utf16le();
	}
	else if(utf_32le_bom::is(rb.position().get()) ) {
		rb.shift(utf_32le_bom::len());
		ch = code_pages::utf32le();
	}
	else if(utf_32be_bom::is(rb.position().get())) {
		rb.shift(utf_32be_bom::len());
		ch = code_pages::utf32be();
	}
	else {
		// No BOM. Now detect stream character set by expensive charset detector
		s_charset_detector chdet = charset_detector::create(ec);
		if(!ec) {
			auto ch_detect_status = chdet->detect(ec, rb.position().get(), rb.length());
			if(!ec && !ch_detect_status )
				ec = make_error_code(converrc::not_supported);
			else
				ch = ch_detect_status.character_set();
		}
	}
	if(!ec) {
		// Open a to UTF-8 converter, if needed. Latin 1 code pages compatiables with UTF-8 and we can continue as is
		s_read_channel text_channel = charset_utf8_compatiable(ch) ? src : open_convert_channel(ec, rb, ch, src);
		if(!ec) {
			source *px = new (std::nothrow) source( std::move(text_channel), std::move(rb) );
			if(nullptr == px)
				ec = std::make_error_code(std::errc::not_enough_memory);
			else
				ret.reset(px, true);
		}
	}
	return ret;
}

s_source source::create(std::error_code& ec,s_read_channel&& src) noexcept
{
	s_source ret;
	byte_buffer buff = byte_buffer::allocate(ec,READ_BUFF_INITIAL_SIZE);
	if(!ec) {
		// charge buffer to detect character set
		uint8_t *pos = const_cast<uint8_t*>(buff.position().get());
		size_t read = src->read(ec, pos, buff.capacity() );
		if(!ec) {
			buff.move(read);
			buff.flip();
			ret = open(ec, src, std::move(buff));
		}
	}
	return ret;
}


source::source(s_read_channel&& src, byte_buffer&& rb) noexcept:
	object(),
	last_( error::ok ),
	pos_(nullptr),
	end_(nullptr),
	row_(1),
	col_(1),
	src_(src),
	rb_( std::move(rb) ),
	mb_state_( 0 )
{
	pos_ = rb_.position().cdata();
	end_ = rb_.last().cdata();
}

source::~source() noexcept
{}

error source::read_more() noexcept
{
	rb_.clear();
//	if( rb_.capacity() < READ_BUFF_MAXIMAL_SIZE ) {
//		if( io_unlikely( !rb_.exp_grow() ) )
//			return error::out_of_memory;
//	}
	std::error_code ec;
	uint8_t* pos = const_cast<uint8_t*>(rb_.position().get());
	size_t read = src_->read(ec, pos, rb_.capacity());
	if(! ec ) {
		rb_.move(read);
		rb_.flip();
	}
	else
		return error::io_error;
	return error::ok;
}

error source::charge() noexcept
{
	error ec = read_more();
	if( io_likely( ec == error::ok && !rb_.empty()) ) {
		pos_ = rb_.position().cdata();
		end_ = rb_.last().cdata();
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
		// according xml standard \r\n combination should be interpret as single \n
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

inline bool source::fetch() noexcept
{
	if( end_ == (pos_+1) )
		last_ = charge();
	return pos_ != end_ || error::ok == last_;
}

char source::next() noexcept
{
	constexpr const char EOF_CH = std::char_traits<char>::to_char_type( std::char_traits<char>::eof() );
	if( io_unlikely( !fetch() ) )
		return EOF_CH;
	char ret;
	// check for a multi-byte tail byte
	if( io_unlikely(0 != mb_state_) ) {
		ret = *pos_++;
		--mb_state_;
	}
	else {
		unsigned int len = utf8::mblen( pos_ );
		ret = *pos_++;
		switch( len ) {
		case io_likely(1):
			ret = normalize_line_endings( ret );
			break;
		case 2:
		case 3:
		case 4:
			mb_state_ = static_cast<uint8_t>(len - 1);
			++col_;
			break;
		default:
			last_ = error::illegal_chars;
			ret = EOF_CH;
			break;
		}
	}
	return ret;
}

void source::read_until_span(byte_buffer& to,const char* span,const std::size_t span_size) noexcept
{
	char c;
	do {
		c = next();
		if( !to.put(c) ) {
			if( io_likely( to.exp_grow() ) )
				to.put(c);
			else
				last_ = error::out_of_memory;
		}
	}
	// we also need ability to handle '\0' char, so use memchr instead of strchr
	while( nullptr == io_memchr(span, c, span_size) && error::ok == last_ );
	to.flip();
}

void source::read_until_char(byte_buffer& to,const char lookup,const char* illegals) noexcept
{
	const std::size_t illegals_len = io_strlen(illegals);
	const std::size_t stops_size = illegals_len + 2;
	char stops[ 16 ] = {'\0'};
	stops[0] = lookup;
	// we need '\0' from illegals
	io_memmove(stops+1, illegals, illegals_len);
	read_until_span(to, stops, stops_size);
	char c = * ( to.last().cdata() - 2 );
	if( lookup != c ) {
		if( is_eof(c) )
			last_ = error::illegal_markup;
		to.clear();
	}
	to.flip();
}

static inline uint16_t pack_word(uint16_t w,char c) noexcept
{
#ifdef IO_IS_LITTLE_ENDIAN
	return (w << CHAR_BIT) | static_cast<uint16_t>(c);
#else
	return (static_cast<uint16_t>(c) >> CHAR_BIT) | w;
#endif // IO_IS_LITTLE_ENDIAN
}

void source::read_until_double_char(byte_buffer& to, const char ch) noexcept
{
	const uint16_t pattern = pack_word(static_cast<uint16_t>(ch), ch);
	uint16_t i = 0;
	char c;
	do {
		c = next();
		if( !to.put(c) ) {
			if( io_likely( to.exp_grow() ) ) {
				to.put(c);
			}
			else {
				last_ = error::out_of_memory;
			}
		}
		i = pack_word(i, c );
	} while( (i != pattern) && not_eof(c) && (error::ok == last_) );
	if( error::ok != last_ || is_eof(c) )
		to.clear();
}

bool source::skip_until_dobule_char(const char ch) noexcept
{
	const uint16_t pattern = pack_word(static_cast<uint16_t>(ch), ch);
	char c;
	uint16_t i = 0;
	do {
		c = next();
		i = pack_word(i,c);
	}
	while( (i != pattern) && not_eof(c) && (error::ok == last_) );
	return i == pattern;
}

} // namespace xml

} // namesapce io

