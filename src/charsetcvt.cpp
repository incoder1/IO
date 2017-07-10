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
#include "charsetcvt.hpp"

#include <iconv.h>

namespace io {

namespace detail {

static const ::iconv_t INVALID_ICONV_DSPTR = reinterpret_cast<::iconv_t>(-1);
static const std::size_t ICONV_ERROR = static_cast<std::size_t>(-1);

static inline converrc iconv_to_conv_errc(int erno) {
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

engine& engine::operator=(engine&& rhs) noexcept {
	engine( std::forward<engine>(rhs) ).swap( *this );
	return *this;
}

bool engine::is_open() const {
	return INVALID_ICONV_DSPTR != iconv_;
}

inline void engine::swap(engine& other) noexcept {
	std::swap(iconv_, other.iconv_);
}

engine::engine() noexcept:
	iconv_( INVALID_ICONV_DSPTR )
{}

engine::engine(const char* from,const char* to):
	iconv_(INVALID_ICONV_DSPTR)
{
	iconv_ = ::iconv_open( to, from );
}

engine::~engine() noexcept
{
	if(INVALID_ICONV_DSPTR != iconv_) {
		::iconv_close(iconv_);
	}
}

converrc engine::convert(uint8_t** src,std::size_t& size, uint8_t** dst, std::size_t& avail) const noexcept
{
	char **s = reinterpret_cast<char**>(src);
	char **d = reinterpret_cast<char**>(dst);
	if( ICONV_ERROR == ::iconv(iconv_, s, &size, d, &avail) ) {
		return iconv_to_conv_errc(errno);
	}
	return converrc::success;
}

} // namesapase detail

// chconv_error_category

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
		return "Destination buffer is to small to trascode all characters";
	case converrc::invalid_multibyte_sequence:
		return "Invalid multi-byte sequence";
	case converrc::incomplete_multibyte_sequence:
		return "Incomplete multi-byte sequence";
	case converrc::not_supported:
		return "Conversion between provided code-pages is not supported";
	case converrc::unknown:
		break;
	}
	return "Character conversion error";
}

std::error_code IO_PUBLIC_SYMBOL make_error_code(io::converrc ec) noexcept
{
	return std::error_code( ec, *(chconv_error_category::instance()) );
}

std::error_condition IO_PUBLIC_SYMBOL make_error_condition(io::converrc err) noexcept
{
	return std::error_condition(static_cast<int>(err), *(chconv_error_category::instance()) );
}

static uint8_t u8_up_rate(const charset& dst_ch) {
	switch( dst_ch.char_max_size() )
	{
		case 1:
			return 0;
		case 2:
			return 1;
	}
	return 2;
}

static bool get_buffer_scale_op(uint8_t& rate,const charset& from,const charset& to)
{
	bool result = false;
	if(from == code_pages::UTF_8) {
		 // to 1 byte, same size
		 // to UCS-2 = size*2
		 // to UCS-4 = size*4
		 rate = u8_up_rate(to);
	} else if(to == code_pages::UTF_8) {
		// if from UCS[2,4] same or less, utherwise up to 4 times
		// larger
		rate = from.unicode() ? 0 : 2;
	} else if( from.char_max_size() == to.char_max_size() ) {
		rate = 0;
	} else if(from.char_max_size() < to.char_max_size()) {
		rate = (from.char_max_size() == 2) ? 1: 2;
	} else {
		result = true;
		rate = (to.char_max_size() == 2) ? 1: 2;
	}
	return result;
}

s_code_cnvtr code_cnvtr::open(std::error_code& ec, const charset& from,const charset& to) noexcept {
	if( from.code() == to.code() ) {
		ec = make_error_code(converrc::not_supported);
		return s_code_cnvtr();
	}
	uint8_t rate;
	bool rhs = get_buffer_scale_op(rate, from, to);
	detail::engine iconve( from.name(), to.name() );
	if(!iconve) {
		ec = make_error_code(converrc::not_supported);
		return s_code_cnvtr();
	}
	code_cnvtr* result = io::nobadalloc<code_cnvtr>::construct( ec, rate, rhs, std::move(iconve)  );
	return !ec ? s_code_cnvtr( result ) : s_code_cnvtr();
}

code_cnvtr::code_cnvtr(uint8_t rate,bool rhs,detail::engine&& eng) noexcept:
	object(),
	eng_( std::forward<detail::engine>(eng) ),
	rate_(rate),
	rhs_(rhs)
{}

void code_cnvtr::convert(std::error_code& ec, uint8_t** in,std::size_t& in_bytes_left,uint8_t** const out, std::size_t& out_bytes_left) const noexcept
{
	converrc result = eng_.convert(in, in_bytes_left, out, out_bytes_left);
	if( converrc::success != result ) {
		ec = make_error_code(result);
		return;
	}
}

void code_cnvtr::convert(std::error_code& ec, const uint8_t* src,const std::size_t size, byte_buffer& dst) const noexcept {
	dst.clear();
	std::size_t left = size;
	std::size_t available = dst.capacity();
	uint8_t** s = const_cast<uint8_t**>(&src);
	const uint8_t* d = dst.position().get();
	while(!ec && left > 0) {
		convert(ec, s, left, const_cast<uint8_t**>(&d), available);
	}
	dst.move(dst.capacity() - available);
	dst.flip();
}

// free functions
std::size_t IO_PUBLIC_SYMBOL transcode(std::error_code& ec, const uint8_t* u8_src, std::size_t src_bytes, char16_t* const dst, std::size_t dst_size) noexcept
{
	assert(nullptr != u8_src && src_bytes > 0);
	assert(nullptr != dst && dst_size > 0);
	static detail::engine eng("UTF-8","UCS-2-INTERNAL");
	uint8_t* s = const_cast<uint8_t*>( reinterpret_cast<const uint8_t*>(u8_src) );
	uint8_t* d = reinterpret_cast<uint8_t*>(dst);
	std::size_t left = src_bytes;
	std::size_t avail = dst_size * sizeof(char16_t);
	converrc result = eng.convert(&s,left,&d,avail);
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
	static detail::engine eng("UTF-8","UCS-4-INTERNAL");
	uint8_t* s = const_cast<uint8_t*>( reinterpret_cast<const uint8_t*>(u8_src) );
	uint8_t* d = reinterpret_cast<uint8_t*>(dst);
	std::size_t left = src_bytes;
	std::size_t avail = dst_size * sizeof(char32_t);
	converrc result = eng.convert(&s,left,&d,avail);
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
	static detail::engine eng("UCS-2-INTERNAL","UTF-8");
	uint8_t* s = const_cast<uint8_t*>( reinterpret_cast<const uint8_t*>(u16_src) );
	uint8_t* d = const_cast<uint8_t*>(u8_dst);
	std::size_t left = src_width * sizeof(char16_t);
	std::size_t avail = dst_size;
	converrc result = eng.convert(&s,left,&d,avail);
	if( converrc::success != result ) {
		ec = make_error_code(result);
		return 0;
	}
	return dst_size - avail;
}

std::size_t IO_PUBLIC_SYMBOL transcode(std::error_code& ec,const char32_t* u32_src, std::size_t src_width, uint8_t* const u8_dst, std::size_t dst_size) noexcept
{
	assert(nullptr != u8_dst && dst_size > 0);
	static detail::engine eng("UCS-4-INTERNAL","UTF-8");
	uint8_t* s = const_cast<uint8_t*>( reinterpret_cast<const uint8_t*>(u32_src) );
	uint8_t* d = const_cast<uint8_t*>(u8_dst);
	std::size_t left = src_width * sizeof(char32_t);
	std::size_t avail = dst_size;
	converrc result = eng.convert(&s,left,&d,avail);
	if( converrc::success != result ) {
		ec = make_error_code(result);
		return 0;
	}
	return dst_size - avail;
}

} // namespace io