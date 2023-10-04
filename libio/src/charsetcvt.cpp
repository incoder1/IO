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
#include "charsetcvt.hpp"
#include "string_algs.hpp"

#include <iconv.h>

// for non GNU libiconv, gnu libc iconv for example
#ifndef iconvctl

#define ICONV_SET_DISCARD_ILSEQ   4  /* const int *argument */

static int iconvctl (iconv_t cd, int request, void* argument)
{
    return 1;
}

#endif // LIBICONV_PLUG

namespace io {

namespace detail {

// engine
static const ::iconv_t INVALID_ICONV_DSPTR = reinterpret_cast<::iconv_t>(-1);
static const std::size_t ICONV_ERROR = static_cast<std::size_t>(-1);

static inline converrc iconv_to_conv_errc(int erno)
{
	switch(erno) {
	case 0:
		return converrc::success;
	case E2BIG:
		return converrc::no_buffer_space;
	case EILSEQ:
		return converrc::invalid_multibyte_sequence;
	case EINVAL:
		return converrc::incomplete_multibyte_sequence;
	default:
		return converrc::unknown;
	}
}

engine::engine(engine&& other) noexcept:
	iconv_(other.iconv_)
{
	other.iconv_ = INVALID_ICONV_DSPTR;
}

engine& engine::operator=(engine&& rhs) noexcept
{
	engine( std::forward<engine>(rhs) ).swap( *this );
	return *this;
}

bool engine::is_open() const
{
	return INVALID_ICONV_DSPTR != iconv_;
}

inline void engine::swap(engine& other) noexcept
{
	std::swap(iconv_, other.iconv_);
}

engine::engine() noexcept:
	iconv_( INVALID_ICONV_DSPTR )
{}

engine::engine(const char* from,const char* to, cnvrt_control control) noexcept:
	iconv_(INVALID_ICONV_DSPTR)
{
	iconv_ = ::iconv_open( to, from );
	if(INVALID_ICONV_DSPTR != iconv_) {
		int discard = control == cnvrt_control::discard_on_failing_chars? 1: 0;
		::iconvctl(iconv_, ICONV_SET_DISCARD_ILSEQ, &discard);
	}
}

engine::~engine() noexcept
{
	if(INVALID_ICONV_DSPTR != iconv_)
		::iconv_close(iconv_);
}

converrc engine::convert(const uint8_t** src,std::size_t& size, uint8_t** dst, std::size_t& avail) const noexcept
{
#ifdef _MSC_VER
	const char **s = reinterpret_cast<const char**>(src);
#else
	char **s = const_cast<char**>( reinterpret_cast<const char**>(src) );
#endif
	char **d = reinterpret_cast<char**>(dst);
	if( ICONV_ERROR == ::iconv(iconv_, s, std::addressof(size), d, std::addressof(avail) ) )
		return iconv_to_conv_errc(errno);
	return converrc::success;
}

// free functions

std::size_t IO_PUBLIC_SYMBOL utf16_buff_size(const char* b, std::size_t size) noexcept
{
	const char *end = b + size;
	std::size_t ret = 0;
	const char *c = b;
	while( (b < end) && '\0' != *c) {
		unsigned int mblen = utf8::mblen(c);
		ret = ret > 2 ? ret + 2 : ret + 1;
		c = c + mblen;
	}
	return ret;
}

std::size_t IO_PUBLIC_SYMBOL utf32_buff_size(const char* b, std::size_t size) noexcept
{
	const char *end = b + size;
	std::size_t ret = 0;
	const char *c = b;
	while( (b < end) && '\0' != *c) {
		unsigned int mblen = utf8::mblen(c);
		ret += mblen;
		c = c + mblen;
	}
	return ret;
}


static constexpr uint16_t USC1_MAX = 0x007F;
static constexpr uint16_t USC2_MAX = 0x07FF;
static constexpr uint16_t USC3_MAX = 0xFFFF;

std::size_t IO_PUBLIC_SYMBOL utf8_buff_size(const char16_t* ustr, std::size_t size) noexcept
{
	std::size_t ret = 0;
	std::size_t i = 0;
	while(i < size) {
		if (ustr[i] <= USC1_MAX) {
			ret += 1;
			++i;
		} else if (ustr[i] <= USC2_MAX) {
			ret += 2;
			++i;
		} else if(ustr[i] <= USC3_MAX) {
			ret += 3;
			++i;
		} else {
			ret += 4;
			i += 2;
		}
	}
	return ret;
}

std::size_t IO_PUBLIC_SYMBOL utf8_buff_size(const char32_t* ustr, std::size_t size) noexcept
{
	std::size_t ret = 0;
	for(std::size_t i = 0; i < size; i++) {
		if(ustr[i] <= USC1_MAX)
			++ret;
		else if(ustr[i] <= USC2_MAX)
			ret += 2;
		else if(ustr[i] <= USC3_MAX)
			ret += 3;
		else
			ret += 4;
	}
	return ret;
}

} // namesapase detail

namespace utf8 {

static void mbtochar32_noshift(char32_t& dst, const char* src,std::size_t& len)
{
	len = 0;
	switch( mblen(src) ) {
	case 1:
		dst = static_cast<char32_t>( *src );
		len = 1;
		break;
	case 2:
		dst = decode2( src );
		len = 2;
		break;
	case 3:
		dst = decode3( src );
		len = 3;
		break;
	case 4:
		dst = decode4( src );
		len = 4;
		break;
	default:
		dst = U'\0';
		break;
	}
}

#ifdef _MSC_VER
IO_PUBLIC_SYMBOL const char* mbtochar32(char32_t& dst, const char* src) noexcept
#else
const char* IO_PUBLIC_SYMBOL mbtochar32(char32_t& dst, const char* src) noexcept
#endif
{
	std::size_t shift;
	mbtochar32_noshift(dst, src, shift);
	return src+shift;
}

#ifdef _MSC_VER
IO_PUBLIC_SYMBOL const u8char_t* mbtochar32(char32_t& dst, const u8char_t* src) noexcept
#else
const u8char_t* IO_PUBLIC_SYMBOL mbtochar32(char32_t& dst, const u8char_t* src) noexcept
#endif
{
	std::size_t shift;
	mbtochar32_noshift(dst, reinterpret_cast<const char*>(src), shift);
	return src+shift;
}

} // namespace utf8

// chconv_error_category

const chconv_error_category* chconv_error_category::instance()
{
	static chconv_error_category _instance;
	return &_instance;
}

const char* chconv_error_category::name() const noexcept
{
	return "Character set conversation error";
}

std::error_condition chconv_error_category::default_error_condition (int err) const noexcept
{
	return std::error_condition(err, *instance() );
}

bool chconv_error_category::equivalent (const std::error_code& code, int condition) const noexcept
{
	return static_cast<int>(this->default_error_condition(code.value()).value()) == condition;
}

const char* chconv_error_category::cstr_message(int err_code) const
{
	converrc ec = static_cast<converrc>(err_code);
	switch( ec ) {
	case converrc::success:
		return "No error";
	case converrc::no_buffer_space:
		return "Destination buffer is to small to transcode all characters";
	case converrc::invalid_multibyte_sequence:
		return "Invalid multi-byte sequence";
	case converrc::incomplete_multibyte_sequence:
		return "Incomplete multi-byte sequence";
	case converrc::not_supported:
		return "Conversion between provided code-pages is not supported";
	case converrc::unknown:
#ifdef LIBICONV_PLUG
	io_unreachable
#endif
		break;
	}
#ifdef LIBICONV_PLUG
	io_unreachable
#endif
	return "Character conversion error";
}


#ifdef __IO_WINDOWS_BACKEND__

static constexpr const char * SYSTEM_UTF16 = "UCS-2-INTERNAL";
static constexpr const char * SYSTEM_UTF32 = "UCS-4-INTERNAL";

#else // unix

#	ifdef IO_IS_LITTLE_ENDIAN
	static constexpr const char * SYSTEM_UTF16 = "UTF-16LE";
	static constexpr const char * SYSTEM_UTF32 = "UTF-32LE";
#	else
	static constexpr const char * SYSTEM_UTF16 = "UTF-16BE";
	static constexpr const char * SYSTEM_UTF32 = "UTF-32BE";
#	endif // IO_IS_LITTLE_ENDIAN

#endif // __IO_WINDOWS_BACKEND__

#ifdef IO_DELCSPEC
IO_PUBLIC_SYMBOL std::error_code make_error_code(io::converrc ec) noexcept
#else
std::error_code IO_PUBLIC_SYMBOL make_error_code(io::converrc ec) noexcept
#endif
{
	return std::error_code( ec, *(chconv_error_category::instance()) );
}

#ifdef IO_DELCSPEC
IO_PUBLIC_SYMBOL std::error_condition make_error_condition(io::converrc err) noexcept
#else
std::error_condition IO_PUBLIC_SYMBOL make_error_condition(io::converrc err) noexcept
#endif
{
	return std::error_condition(static_cast<int>(err), *(chconv_error_category::instance()) );
}

s_code_cnvtr code_cnvtr::open(std::error_code& ec,const charset& from,const charset& to,cnvrt_control control) noexcept
{
	if( !from || !to || from == to ) {
		ec = make_error_code(converrc::not_supported);
		return s_code_cnvtr();
	}
	detail::engine iconve( from.name(), to.name(), control);
	if(!iconve) {
		ec = make_error_code(converrc::not_supported);
		return s_code_cnvtr();
	}
	code_cnvtr* result = io::nobadalloc<code_cnvtr>::construct( ec, std::move(iconve)  );
	return !ec ? s_code_cnvtr( result ) : s_code_cnvtr();
}

code_cnvtr::code_cnvtr(detail::engine&& eng) noexcept:
	object(),
	eng_( std::forward<detail::engine>(eng) )
{
}

void code_cnvtr::convert(std::error_code& ec, const uint8_t** in,std::size_t& in_bytes_left,uint8_t** const out, std::size_t& out_bytes_left) const noexcept
{
	converrc result = eng_.convert(in, in_bytes_left, out, out_bytes_left);
	if( converrc::success != result ) {
		ec = make_error_code(result);
		return;
	}
}

void code_cnvtr::convert(std::error_code& ec, const uint8_t* src,const std::size_t size, byte_buffer& dst) const noexcept
{
	dst.clear();
	std::size_t left = size;
	std::size_t available = dst.available();
	const uint8_t** s = std::addressof(src);
	uint8_t* d = const_cast<uint8_t*>( dst.position().get() );
	while(!ec && left > 0)
		convert(ec, s, left, std::addressof(d), available);
	dst.move(dst.capacity() - available);
	dst.flip();
}

// free functions
std::size_t IO_PUBLIC_SYMBOL transcode(std::error_code& ec, const uint8_t* u8_src, std::size_t src_bytes, char16_t* const dst, std::size_t dst_size) noexcept
{
	assert(nullptr != u8_src && src_bytes > 0);
	assert(nullptr != dst && dst_size > 0);
	static detail::engine eng(
								code_pages::UTF_8.name(),
								SYSTEM_UTF16,
								cnvrt_control::failure_on_failing_chars
							);
	uint8_t* d = reinterpret_cast<uint8_t*>(dst);
	std::size_t left = src_bytes;
	std::size_t avail = dst_size * sizeof(char16_t);
	converrc result = eng.convert(&u8_src,left,&d,avail);
	if( converrc::success != result ) {
		ec = make_error_code(result);
		return 0;
	}
	return dst_size - (avail / sizeof(char16_t) );
}

std::size_t IO_PUBLIC_SYMBOL transcode(std::error_code& ec,const uint8_t* u8_src, std::size_t src_bytes, char32_t* const dst, std::size_t dst_size) noexcept
{
	assert(nullptr != u8_src && src_bytes > 0);
	assert(nullptr != dst && dst_size > 1);
	static detail::engine eng(code_pages::UTF_8.name(),
							SYSTEM_UTF32,
							cnvrt_control::failure_on_failing_chars);
	uint8_t* d = reinterpret_cast<uint8_t*>(dst);
	std::size_t left = src_bytes;
	std::size_t avail = dst_size * sizeof(char32_t);
	converrc result = eng.convert( &u8_src,left, &d, avail);
	if( converrc::success != result ) {
		ec = make_error_code(result);
		return 0;
	}
	return dst_size - (avail / sizeof(char32_t) );
}

std::size_t IO_PUBLIC_SYMBOL transcode(std::error_code& ec,const char16_t* u16_src, std::size_t src_width, uint8_t* const u8_dst, std::size_t dst_size) noexcept
{
	assert(nullptr != u16_src && src_width > 0);
	assert(nullptr != u8_dst && dst_size > 1);
	static detail::engine eng(
							SYSTEM_UTF16,
							code_pages::UTF_8.name(),
							cnvrt_control::failure_on_failing_chars);
	const uint8_t* s = reinterpret_cast<const uint8_t*>(u16_src);
	uint8_t* d = const_cast<uint8_t*>(u8_dst);
	std::size_t left = src_width * sizeof(char16_t);
	std::size_t avail = dst_size;
	converrc result = eng.convert( &s ,left, &d, avail);
	if( converrc::success != result ) {
		ec = make_error_code(result);
		return 0;
	}
	return dst_size - avail;
}

std::size_t IO_PUBLIC_SYMBOL transcode(std::error_code& ec,const char32_t* u32_src, std::size_t src_width, uint8_t* const u8_dst, std::size_t dst_size) noexcept
{
	assert(nullptr != u8_dst && dst_size > 0);
	static detail::engine eng(SYSTEM_UTF32,
							code_pages::UTF_8.name(),
							cnvrt_control::failure_on_failing_chars);
	const uint8_t* s =  reinterpret_cast<const uint8_t*>(u32_src);
	uint8_t* d = const_cast<uint8_t*>(u8_dst);
	std::size_t left = src_width * sizeof(char32_t);
	std::size_t avail = dst_size;
	converrc result = eng.convert( &s,left, &d,avail);
	if( converrc::success != result ) {
		ec = make_error_code(result);
		return 0;
	}
	return dst_size - avail;
}

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
	const std::size_t length = bytes - (bytes % 2);
	const std::size_t rdbuflen = length >> 2;
	uint8_t* rdbuf;
	if ( rdbuflen <= MAX_CONVB_STACK_SIZE) {
		rdbuf = static_cast<uint8_t*>( io_alloca(rdbuflen) );
	} else {
		rdbuf = memory_traits::malloc_array<uint8_t>( rdbuflen );
		if(nullptr == rdbuf) {
			ec = std::make_error_code(std::errc::not_enough_memory);
			return 0;
		}
	}
	std::size_t left = length;
	std::size_t read;
	do {
		read = src_->read(ec, rdbuf, rdbuflen);
		if( 0 == read || ec)
			break;
		do {
			conv_->convert(ec, const_cast<const uint8_t**>(&rdbuf), read, const_cast<uint8_t**>(&buff), left);
		} while(read > 0 && !ec);
	} while(left > 0);
	if(rdbuflen > MAX_CONVB_STACK_SIZE)
		memory_traits::free( rdbuf );
	else
		io_freea( rdbuf );
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
	const std::size_t to_convert = (size << 2);
	std::size_t left_after = to_convert;
	const uint8_t** uncv = std::addressof(src);
	uint8_t** conv = std::addressof(dst);
	conv_->convert(ec, uncv, size, conv, left_after);
	return to_convert - left_after;
}

std::size_t conv_write_channel::write(std::error_code& ec, const uint8_t* buff,std::size_t bytes) const noexcept
{
	// considering worst scenario with up bytes conversion, i.e. something like UTF-8 to UTF-32
	const std::size_t cnvbuflen = bytes << 2;
	uint8_t* cnvbuff;
	// don't touch heap, until not needed
	if( cnvbuflen <= MAX_CONVB_STACK_SIZE ) {
		cnvbuff = static_cast<uint8_t*>( io_alloca( cnvbuflen ) );
	} else {
		cnvbuff = memory_traits::malloc_array<uint8_t>( cnvbuflen );
		if(nullptr == cnvbuff) {
			ec = std::make_error_code(std::errc::not_enough_memory);
			return 0;
		}
	}

	std::size_t unconv_left = bytes;
	std::size_t to_write = convert_some(ec, buff, unconv_left, cnvbuff);
	// check for the transcoding error
	std::size_t ret = io_unlikely( ec ) ? 0 : (bytes - unconv_left);
	if( ret > 0 ) {
		// write all converted bytes to the destination stream
		uint8_t *wpos = cnvbuff;
		std::size_t written;
		do {
			written = dst_->write(ec, wpos, to_write);
			wpos += written;
			to_write -= written;
		} while( to_write > 0 && !ec );
		if( ec )
			ret = 0;
	}

	if(cnvbuflen > MAX_CONVB_STACK_SIZE)
		memory_traits::free( cnvbuff );

	return ret;
}

} // namespace io
