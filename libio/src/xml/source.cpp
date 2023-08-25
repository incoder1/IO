/*
 *
 * Copyright (c) 2016-2022
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#include "stdafx.hpp"
#include "source.hpp"
#include "string_algs.hpp"

namespace io {

namespace xml {

const std::size_t source::READ_BUFF_INITIAL_SIZE = memory_traits::page_size(); // 4k in most cases
const std::size_t source::READ_BUFF_MAXIMAL_SIZE = source::READ_BUFF_INITIAL_SIZE * source::READ_BUFF_INITIAL_SIZE; // about 16 mb

static constexpr unsigned int ASCII_CP_CODE = 20127;
static constexpr unsigned int ISO_LATIN1_CP_CODE = 28591;
static constexpr unsigned int WINDOWS_LATIN1_CP_CODE = 1252;
static constexpr unsigned int UTF8_CP_CODE = 65001;

static constexpr char NL = '\n';
static constexpr char CR = '\r';


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

void source::read_until_char(byte_buffer& to,const char lookup,const char* illegals) noexcept
{
	const std::size_t illegals_len = io_strlen(illegals);
	const std::size_t stops_size = illegals_len + 2;
	char stops[ 16 ] = {'\0'};
	stops[0] = lookup;
	// we need '\0' from illegals
	io_memmove(stops+1, illegals, illegals_len);
	char c;
	do {
		c = next();
		if( !to.put(c) ) {
			if( io_likely( to.exp_grow() ) ) {
				to.put(c);
			}
			else {
				last_ = error::out_of_memory;
				break;
			}
		}
	}
	while( nullptr == io_memchr(stops, c, stops_size) );
	if( lookup != c ) {
		if( is_eof(c) )
			last_ = error::illegal_markup;
		to.clear();
	}
	to.flip();
}

constexpr uint16_t pack_word(uint16_t w, uint16_t c) noexcept
{
#ifdef IO_IS_LITTLE_ENDIAN
	return (w << CHAR_BIT) | c;
#else
	return (c >> CHAR_BIT) | w;
#endif // IO_IS_LITTLE_ENDIAN
}

void source::read_until_double_char(byte_buffer& to, const char ch) noexcept
{
	const uint16_t pattern = pack_word(static_cast<uint16_t>(ch), ch);
	char c;
	uint16_t i = 0;
	do {
		c = next();
		if( io_unlikely( cheq(std::char_traits<char>::eof(), c)) )
			break;
		if( !to.put(c) ) {
			if( io_likely( to.exp_grow() ) ) {
				to.put(c);
			}
			else {
				last_ = error::out_of_memory;
				break;
			}
		}
		i = pack_word(i, static_cast<uint16_t>(c) );
	}
	while( i != pattern);
	if( error::ok != last_ || is_eof(c) )
		to.clear();
}

void source::skip_until_dobule_char(const char ch) noexcept
{
	const uint16_t pattern = pack_word(static_cast<uint16_t>(ch), ch);
	char c;
	uint16_t i = 0;
	do {
		c = next();
		if( io_unlikely( is_eof(c) ) )
			break;
		i = pack_word(i,c);
	}
	while( i != pattern);
}

} // namespace xml

} // namesapce io

