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
#ifndef __IO_STREAMS_HPP_INCLUDED__
#define __IO_STREAMS_HPP_INCLUDED__

#include "config.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include "errorcheck.hpp"
#include "channels.hpp"

#include <streambuf>
#include <ostream>

namespace io {

namespace {

	void ios_check_error_code(const char* msg, std::error_code const &ec )
	{
		#ifdef IO_NO_EXCEPTIONS
			std::string message(msg);
			message.push_back(' ');
			message.append(ec.message());
			io::detail::panic(ec.value(), message.c_str() );
		#else
			throw std::ios_base::failure( msg, ec );
		#endif
	}
}

template<typename __char_type, class __traits_type >
class ochannel_streambuf final: public std::basic_streambuf<__char_type, __traits_type >
{
	ochannel_streambuf(const ochannel_streambuf&) = delete;
	ochannel_streambuf operator=(const ochannel_streambuf&) = delete;
private:
	typedef std::basic_streambuf<__char_type, __traits_type > super_type;
	typedef ochannel_streambuf<__char_type, __traits_type> selt_type;

	void clear()
	{
		super_type::setp( data_, const_cast<char_type*>(end_) );
	}

	std::streamsize write_buffer()
	{
		std::error_code ec;
		const uint8_t *ptr = reinterpret_cast<uint8_t*>( data_ );
		const uint8_t *end = reinterpret_cast<uint8_t*>( this->pptr() );
		std::size_t bytes_written;
		do {
			bytes_written = ch_->write(ec, ptr, memory_traits::distance(ptr,end) );
			if(ec)
				return traits_type::eof();
			ptr += bytes_written;
		} while( ptr > end);
		clear();
		return 1;
	}

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

	ochannel_streambuf(s_write_channel&& ch,const std::size_t buffer_size):
		super_type(),
		ch_( std::forward<s_write_channel>(ch)),
		data_(nullptr),
		end_(nullptr)
	{
		data_ = static_cast<char_type*>( memory_traits::malloc( buffer_size * sizeof(char_type) ) );
		if(nullptr == data_ ) {
			std::error_code ec = std::make_error_code(std::errc::not_enough_memory);
			ios_check_error_code( "output stream buff ", ec );
		}
		end_ = data_ + buffer_size;
		clear();
	}

	ochannel_streambuf(const s_write_channel& ch,const std::size_t buffer_size):
		ochannel_streambuf(
		    s_write_channel( ch ),
		    buffer_size
		)
	{}

	virtual ~ochannel_streambuf() override
	{
		if(nullptr != data_ ) {
			write_buffer();
			memory_traits::free(data_);
		}
	}


	virtual super_type* setbuf(char_type* data, std::streamsize size) override
	{
		this->xsputn(data, size);
		return this;
	}

	virtual std::streamsize xsputn(const char_type* __s, std::streamsize __n) override
	{
		if(nullptr != __s && __n > 0) {
			char_type* pos = this->pptr();
			std::streamsize avail = this->showmanyc();
			std::streamsize ret;
			if(__n >  avail)
				ret = avail;
			else
				ret = __n;
			std::copy(__s, (__s+__n), pos);
			super_type::pbump( ret );
			return ret;
		}
		return 0;
	}

	virtual int_type overflow(int_type __c) override
	{
		return static_cast<int_type>( write_buffer() );
	}

	virtual std::streamsize showmanyc() override
	{
		return static_cast<std::streamsize>( end_ - this->pptr() );
	}

	virtual int sync() override
	{
		return write_buffer();
	}

public:
	s_write_channel ch_;
	char_type *data_;
	const char_type *end_;
};


template<typename __char_type>
class channel_ostream:public std::basic_ostream<__char_type, std::char_traits<__char_type> > {
	channel_ostream(const channel_ostream&) = delete;
	channel_ostream& operator=(const channel_ostream&) = delete;
private:
	typedef std::basic_ostream<__char_type, std::char_traits<__char_type> > super_type;
	typedef ochannel_streambuf<__char_type, std::char_traits<__char_type> > streambuf_type;
public:
	channel_ostream(s_write_channel&& ch):
		super_type(),
		sb_( new streambuf_type(
		         std::forward<s_write_channel>(ch),
		         memory_traits::page_size() )
		   )
	{
		this->init(sb_);
	}

	channel_ostream(const s_write_channel& ch):
		channel_ostream( s_write_channel(ch) )
	{}

	virtual ~channel_ostream() override
	{
		delete sb_;
	}

private:
	streambuf_type *sb_;
};

template<typename __char_type, class __traits_type >
class ichannel_streambuf final: public std::basic_streambuf<__char_type, __traits_type >
{
	typedef std::basic_streambuf<__char_type, __traits_type > super_type;
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

	ichannel_streambuf(s_read_channel&& src, std::size_t buffer_size):
		rch_( std::forward<s_read_channel>(src) ),
		data_(nullptr)
	{
		data_ = static_cast<char_type*>( memory_traits::malloc( buffer_size * sizeof(char_type) ) );
		if(nullptr == data_ ) {
			std::error_code ec = std::make_error_code(std::errc::not_enough_memory);
			ios_check_error_code( "input stream buff ", ec );
		}
	}

	virtual ~ichannel_streambuf() override
	{
		memory_traits::free( data_ );
	}
private:
	s_read_channel rch_;
	char_type data_;
};

} // namespace io

#endif // __IO_STREAMS_HPP_INCLUDED__
