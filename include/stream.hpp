#ifndef __IO_STREAMS_HPP_INCLUDED__
#define __IO_STREAMS_HPP_INCLUDED__

#include "config.hpp"
#include "channels.hpp"

#include <streambuf>
#include <ostream>

#ifdef IO_NO_EXCEPTIONS
#	include <cstdio>
#endif // IO_NO_EXCEPTIONS

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

namespace io {

namespace {
	static inline void check_error_code(std::error_code& ec) {
		if(ec) {
#ifdef IO_NO_EXCEPTIONS
		std::fprintf(stderr, ec.message().c_str() );
		std::unexpected();
#else
			throw std::system_error(ec);
#endif // IO_NO_EXCEPTIONS
		}
	}
}

template<typename __char_type, class __traits_type >
class ochannel_streambuf: public std::basic_streambuf<__char_type, __traits_type >
{
private:
	typedef std::basic_streambuf<__char_type, __traits_type > super_type;
	typedef ochannel_streambuf<__char_type, __traits_type> selt_type;
public:

	/**
	*  These are standard types.  They permit a standardized way of
	*  referring to names of (or names dependent on) the template
	*  parameters, which are specific to the implementation.
	*/
	typedef typename super_type::char_type char_type;
	typedef typename super_type::traits_type traits_type;
	typedef typename traits_type::int_type int_type;
	typedef typename traits_type::pos_type pos_type;
	typedef typename traits_type::off_type off_type;

	ochannel_streambuf(const s_write_channel& ch, std::streamsize buffer_size):
		super_type(),
		ch_(ch),
		buff_()
	{
		std::error_code ec;
		buff_ = byte_buffer::allocate(ec, static_cast<std::size_t>(buffer_size) );
		check_error_code(ec);
	}

	ochannel_streambuf(s_write_channel&& ch, std::streamsize buffer_size):
		super_type(),
		ch_( std::forward<s_write_channel>(ch)),
		buff_()
	{
		std::error_code ec;
		buff_ = byte_buffer::allocate(ec, buffer_size );
		check_error_code(ec);
	}

	virtual super_type* setbuf(char_type* data, std::streamsize size) override
	{
		std::error_code ec;
		buff_ = byte_buffer::allocate(ec, size);
		check_error_code(ec);
		buff_.put(data, size);
		return this;
	}

	virtual std::streamsize xsputn(const char_type* __s, std::streamsize __n) override
	{
		return static_cast<std::streamsize> (
					buff_.put(__s, static_cast<std::size_t>(__n) )
					);
	}

	virtual int_type overflow(int_type __c) override
	{
		std::error_code ec;
		if( !traits_type::not_eof(__c) )
			traits_type::eof();
		buff_.flip();
		int ret = static_cast<int>( ch_->write(ec, buff_.position().get(), buff_.length()) );
		if(ec)
			return -1;
		return ret;
	}


public:
	s_write_channel ch_;
	byte_buffer buff_;
};


template<typename __char_type>
class channel_ostream:public std::basic_ostream<__char_type, std::char_traits<__char_type> >
{
	channel_ostream(const channel_ostream&) = delete;
	channel_ostream& operator=(const channel_ostream&) = delete;
private:
	typedef std::basic_ostream<__char_type, std::char_traits<__char_type> > super_type;
	typedef ochannel_streambuf<__char_type, std::char_traits<__char_type> > streambuf_type;
	static std::streamsize page_size() {
#ifdef __IO_WINDOWS_BACKEND__
		SYSTEM_INFO si;
		GetSystemInfo(&si);
		return static_cast<std::streamsize>( si.dwPageSize );
#elif defined(__IO_POSIX_BACKEND__)
		return static_cast<std::streamsize>( sysconf(_SC_PAGESIZE) );
#else
		return 4096;
#endif // __IO_WINDOWS_BACKEND__
	}
public:
	channel_ostream(const s_write_channel& ch):
		super_type(),
		sb_( new streambuf_type(ch, page_size() ) )
	{
		this->init(sb_);
	}
	virtual ~channel_ostream() override
	{
		delete sb_;
	}
private:
	streambuf_type *sb_;
};


} // namespace io

#endif // __IO_STREAMS_HPP_INCLUDED__
