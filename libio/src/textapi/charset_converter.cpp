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

#include <iconv.h>

#include "io/core/string_algs.hpp"
#include "io/textapi/charset_converter.hpp"

namespace io {

namespace detail {

// Free functions
template<typename C, typename I = std::make_unsigned<C>::type, typename U = std::make_unsigned< I >::type >
#ifdef IO_HAS_CONNCEPTS
requires( is_charater_v<C> )
#endif // IO_HAS_CONNCEPTS
static constexpr U to_uint(const C c) noexcept
{
	return static_cast< U > ( std::char_traits<C>::to_int_type(c) );
}

static constexpr unsigned int USC1_MAX = 0x7F;
static constexpr unsigned int USC2_MAX = 0x7FF;
static constexpr unsigned int USC3_MAX = 0xFFFF;

#ifdef IO_DELCSPEC
IO_PUBLIC_SYMBOL std::size_t
#else
std::size_t IO_PUBLIC_SYMBOL
#endif // IO_DELCSPEC
utf8_buff_size(const char16_t* ustr, std::size_t size) noexcept
{
	std::size_t ret = 0;
	std::size_t i = 0;
	while(i < size) {
		if ( to_uint(ustr[i]) <= USC1_MAX) {
			ret += 1;
			++i;
		}
		else if ( to_uint(ustr[i]) <= USC2_MAX) {
			ret += 2;
			++i;
		}
		else if( to_uint(ustr[i]) <= USC3_MAX) {
			ret += 3;
			++i;
		}
		else {
			ret += 4;
			i += 2;
		}
	}
	return ret;
}

#ifdef IO_DELCSPEC
IO_PUBLIC_SYMBOL std::size_t
#else
std::size_t IO_PUBLIC_SYMBOL
#endif // IO_DELCSPEC
utf8_buff_size(const char32_t* ustr, std::size_t size) noexcept
{
	std::size_t ret = 0;
	for(std::size_t i = 0; i < size; i++) {
		if( to_uint(ustr[i]) <= USC1_MAX)
			++ret;
		else if( to_uint(ustr[i]) <= USC2_MAX)
			ret += 2;
		else if( to_uint(ustr[i]) <= USC3_MAX)
			ret += 3;
		else
			ret += 4;
	}
	return ret;
}

#ifdef IO_DELCSPEC
IO_PUBLIC_SYMBOL std::size_t
#else
std::size_t IO_PUBLIC_SYMBOL
#endif // IO_DELCSPECxcept;
utf16_buff_size(const char* b, std::size_t size) noexcept
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

#ifdef IO_DELCSPEC
IO_PUBLIC_SYMBOL std::size_t
#else
std::size_t IO_PUBLIC_SYMBOL
#endif // IO_DELCSPEC
utf32_buff_size(const char* b, std::size_t size) noexcept
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

// engine
static const ::iconv_t INVALID_ICONV_DSPTR = reinterpret_cast<::iconv_t>(-1);
static const std::size_t ICONV_ERROR = static_cast<std::size_t>(-1);

static inline converrc iconv_to_conv_errc(int errc) noexcept
{
	switch(errc) {
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

engine::engine(const char* from,const char* to) noexcept:
	iconv_( ::iconv_open( to, from ) )
{}

engine::~engine() noexcept
{
	if(INVALID_ICONV_DSPTR != iconv_)
		::iconv_close(iconv_);
}

converrc engine::convert(const uint8_t** src,std::size_t& size, uint8_t** dst, std::size_t& avail) const noexcept
{
	converrc ret = converrc::success;
#ifdef _MSC_VER
	const char **s = reinterpret_cast<const char**>(src);
#else
	char **s = const_cast<char**>( reinterpret_cast<const char**>(src) );
#endif
	char **d = reinterpret_cast<char**>(dst);
	if( ICONV_ERROR == ::iconv(iconv_, s, std::addressof(size), d, std::addressof(avail) ) )
		ret = iconv_to_conv_errc( errno );
	return ret;
}

} // namespace detail

// chconv_error_category

const char* chconv_error_category::name() const noexcept
{
	return "Character set conversation error";
}

std::error_condition chconv_error_category::default_error_condition (int err) const noexcept
{
	return std::error_condition(err, chconv_error_category() );
}

bool chconv_error_category::equivalent (const std::error_code& code, int condition) const noexcept
{
	return static_cast<int>( this->default_error_condition(code.value()).value() ) == condition;
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

#ifdef IO_IS_LITTLE_ENDIAN
static constexpr const char * SYSTEM_UTF16 = "UTF-16LE";
static constexpr const char * SYSTEM_UTF32 = "UTF-32LE";
#else
static constexpr const char * SYSTEM_UTF16 = "UTF-16BE";
static constexpr const char * SYSTEM_UTF32 = "UTF-32BE";
#endif // IO_IS_LITTLE_ENDIAN

#endif // __IO_WINDOWS_BACKEND__

#ifdef IO_DELCSPEC
IO_PUBLIC_SYMBOL std::error_code
#else
std::error_code IO_PUBLIC_SYMBOL
#endif
make_error_code(io::converrc ec) noexcept
{
	return std::error_code( ec, chconv_error_category() );
}

#ifdef IO_DELCSPEC
IO_PUBLIC_SYMBOL std::error_condition
#else
std::error_condition IO_PUBLIC_SYMBOL
#endif
make_error_condition(io::converrc err) noexcept
{
	return std::error_condition(static_cast<int>(err), chconv_error_category() );
}

s_charset_converter charset_converter::open(std::error_code& ec,const charset* from,const charset* to) noexcept
{
	s_charset_converter ret;
	if( !from || !to || from == to ) {
		ec = make_error_code(converrc::not_supported);
	}
	else {
		detail::engine eng( from->name(), to->name() );
		if( !eng.is_open() ) {
			ec = make_error_code(converrc::not_supported);
		}
		else {
			charset_converter* px = new (std::nothrow) charset_converter( std::move(eng)  );
			if(nullptr == px)
				ec = std::make_error_code(std::errc::not_enough_memory);
			else
				ret.reset(px, true);
		}
	}
	return ret;
}

charset_converter::charset_converter(detail::engine&& eng) noexcept:
	object(),
	eng_( std::forward<detail::engine>(eng) )
{}

void charset_converter::convert(std::error_code& ec, const uint8_t** in,std::size_t& in_bytes_left,uint8_t** const out, std::size_t& out_bytes_left) const noexcept
{
	converrc err_code = eng_.convert(in, in_bytes_left, out, out_bytes_left);
	if( converrc::success != err_code ) {
		ec = make_error_code(err_code);
	}
}

void charset_converter::convert(std::error_code& ec, const uint8_t* src,const std::size_t size, byte_buffer& dst) const noexcept
{
	dst.clear();
	std::size_t left = size;
	std::size_t available = dst.available();
	const uint8_t** s = std::addressof(src);
	uint8_t* d = const_cast<uint8_t*>( dst.position().get() );
	while(left > 0 && !ec) {
		convert(ec, s, left, std::addressof(d), available);
	}
	if(!ec) {
		dst.move(dst.capacity() - available);
		dst.flip();
	}
	else {
		dst.clear();
	}
}

void charset_converter::convert(std::error_code& ec, byte_buffer& src, byte_buffer& dst) const noexcept
{
	const uint8_t* sp = src.position().get();
	const uint8_t** s = std::addressof(sp);
	uint8_t* dp =  const_cast<uint8_t*>( dst.position().get() );
	uint8_t** d = std::addressof( dp );
	std::size_t left = src.length();
	std::size_t available = dst.available();
	convert(ec, s, left, d, available);
	if(!ec) {
		src.shift(src.length() - left);
		dst.move(dst.capacity() - available);
		dst.flip();
	}
	else {
		dst.clear();
	}
}

// transcode functions
std::size_t IO_PUBLIC_SYMBOL transcode(std::error_code& ec, const uint8_t* u8_src, std::size_t src_bytes, char16_t* const dst, std::size_t dst_size) noexcept
{
	assert(nullptr != u8_src && src_bytes > 0);
	assert(nullptr != dst && dst_size > 0);
	static detail::engine eng("UTF-8",SYSTEM_UTF16);
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
	static detail::engine eng("UTF-8",SYSTEM_UTF32);
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
	static detail::engine eng(SYSTEM_UTF16,"UTF-8");
	const uint8_t* s = reinterpret_cast<const uint8_t*>(u16_src);
	uint8_t* d = const_cast<uint8_t*>(u8_dst);
	std::size_t left = src_width * sizeof(char16_t);
	std::size_t avail = dst_size;
	converrc result = eng.convert( &s,left, &d, avail);
	if( converrc::success != result ) {
		ec = make_error_code(result);
		return 0;
	}
	return dst_size - avail;
}

std::size_t IO_PUBLIC_SYMBOL transcode(std::error_code& ec,const char32_t* u32_src, std::size_t src_width, uint8_t* const u8_dst, std::size_t dst_size) noexcept
{
	assert(nullptr != u8_dst && dst_size > 0);
	static detail::engine eng(SYSTEM_UTF32,"UTF-8");
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

// charset_converting_channel_pump
s_pump charset_converting_channel_pump::create(std::error_code& ec, s_read_channel&& src,const charset* from, const charset* to, std::size_t out_buffer_size) noexcept
{
	s_pump ret;
	byte_buffer rb = byte_buffer::allocate(ec, (out_buffer_size / 2) );
	byte_buffer cvb = byte_buffer::allocate(ec, out_buffer_size );
	if(!ec) {
		uint8_t* px = const_cast<uint8_t*>(rb.position().get());
		std::size_t read = src->read(ec, px, rb.capacity());
		if(!ec && read > 0) {
			rb.move(read);
			rb.flip();
			auto cvt = charset_converter::open(ec, from, to);
			if(!ec) {
				cvt->convert(ec, rb, cvb);
				if(!ec) {
					auto px = new (std::nothrow) charset_converting_channel_pump( std::forward<s_read_channel>(src), std::move(rb), std::move(cvb), std::move(cvt) );
					if(nullptr == px)
						ec = std::make_error_code(std::errc::not_enough_memory);
					else
						ret.reset(px, true);
				}
			}
		}
	}
	return ret;
}

charset_converting_channel_pump::charset_converting_channel_pump(s_read_channel&& src, byte_buffer&& rb, byte_buffer&& cvb, s_charset_converter&& cvt) noexcept:
	buffered_channel_pump( std::forward<s_read_channel>(src), std::forward<byte_buffer>(rb) ),
	cvt_buff_(std::forward<byte_buffer>(cvb)),
	cvt_(std::forward<s_charset_converter>(cvt))
{}

bool charset_converting_channel_pump::sync(std::error_code& ec) noexcept
{
	uint8_t* px = const_cast<uint8_t*>(read_buff_.position().get());
	std::size_t read_bytes = channel_pump::pull(ec, px, read_buff_.capacity() );
	bool ret = false;
	if(!ec && read_bytes > 0) {
		read_buff_.flip();
		cvt_buff_.clear();
		cvt_->convert(ec, read_buff_, cvt_buff_);
		ret = !ec;
	}
	return ret;
}

std::size_t charset_converting_channel_pump::pull(std::error_code& ec, uint8_t* const to, std::size_t bytes) noexcept
{
	std::size_t	ret = 0;
	while( (bytes > 0) && !ec) {
		std::size_t buffered = cvt_buff_.length();
		if( buffered >= bytes) {
			buffered = bytes;
		}
		else if( 0 == buffered) {
			if( !sync(ec) )
				break;
		}
		io_memmove(to, cvt_buff_.position().get(), buffered );
		cvt_buff_.shift(buffered);
		ret += bytes;
		bytes -= buffered;
	}
	return ret;
}

// charset_converting_channel_funnel
s_funnel charset_converting_channel_funnel::create(std::error_code& ec, s_write_channel&& dst, const charset* from, const charset* to, std::size_t buffer_size) noexcept
{
	s_funnel ret;
	byte_buffer wb = byte_buffer::allocate(ec, buffer_size );
	byte_buffer cvb = byte_buffer::allocate(ec, (buffer_size / 2) );
	if(!ec) {
		auto cvt = charset_converter::open(ec, from, to);
		if(!ec) {
			charset_converting_channel_funnel *px = new (std::nothrow) charset_converting_channel_funnel( std::forward<s_write_channel>(dst), std::move(wb), std::move(cvb), std::move(cvt) );
			if(nullptr == px)
				ec = std::make_error_code(std::errc::not_enough_memory);
			else
				ret.reset(px, true);
		}
	}
	return ret;
}

charset_converting_channel_funnel::charset_converting_channel_funnel(s_write_channel&& dst, byte_buffer&& wb, byte_buffer&& cvb, s_charset_converter&& cvt) noexcept:
	buffered_channel_funnel( std::forward<s_write_channel>(dst), std::forward<byte_buffer>(wb) ),
	cvt_buff_( std::forward<byte_buffer>(cvb) ),
	cvt_( std::forward<s_charset_converter>(cvt) )
{}

void charset_converting_channel_funnel::flush(std::error_code& ec) noexcept
{
	cvt_buff_.flip();
	while( (cvt_buff_.length() > 0 ) && !ec ) {
		write_buff_.clear();
		cvt_->convert(ec, cvt_buff_, write_buff_);
		if(!ec)
			buffered_channel_funnel::flush(ec);
	}
	if(!ec)
		cvt_buff_.clear();
}

std::size_t charset_converting_channel_funnel::push(std::error_code& ec, const uint8_t* src, std::size_t bytes) noexcept
{
	std::size_t ret = 0;
	const uint8_t* px = src;
	while( !ec &&  (bytes > 0) ) {
		std::size_t available = cvt_buff_.available();
		if( available == 0 ) {
			flush(ec);
		}
		else if( bytes > cvt_buff_.capacity() ) {
			cvt_buff_.put(px, available);
			px += available;
			bytes -= available;
			ret += available;
		}
		else if( bytes <= available) {
			cvt_buff_.put( px, bytes );
			ret += bytes;
			bytes = 0;
		}
	}
	return ret;
}

} // namespace io
