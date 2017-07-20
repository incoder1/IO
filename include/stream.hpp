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
	if(!ec)
		return;
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
class ochannel_streambuf final: public std::basic_streambuf<__char_type, __traits_type > {
	ochannel_streambuf(const ochannel_streambuf&) = delete;
	ochannel_streambuf operator=(const ochannel_streambuf&) = delete;
private:
	typedef std::basic_streambuf<__char_type, __traits_type > super_type;
	typedef ochannel_streambuf<__char_type, __traits_type> self_type;

	void clear() noexcept
	{
		this->setp( this->pbase(), this->epptr() );
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

	ochannel_streambuf(s_write_channel&& ch,std::size_t buffer_size):
		super_type(),
		ch_( std::forward<s_write_channel>(ch) ),
		ref_count_( 0 )
	{
		// allign buffer on sizeof(char_size)
		buffer_size = (buffer_size + (sizeof(char_type) - 1) ) & ~( (sizeof(char_type) - 1) );
		uint8_t *buff = static_cast<uint8_t*>( memory_traits::malloc( buffer_size ) );
		if(nullptr == buff) {
			std::error_code ec = std::make_error_code(std::errc::not_enough_memory);
			ios_check_error_code( "output stream buff ", ec );
		}
		uint8_t *end_buff = buff + buffer_size;
		this->setp( reinterpret_cast<char_type*>(buff), reinterpret_cast<char_type*>(end_buff) );
	}

	ochannel_streambuf(const s_write_channel& ch,const std::size_t buffer_size):
		ochannel_streambuf(
		    s_write_channel( ch ),
		    buffer_size
		)
	{}

	void swap(ochannel_streambuf& __sb)
	{
		ch_.swap( __sb.ch_ );
		super_type::swap( __sb );
	}

	virtual ~ochannel_streambuf() override
	{
		if( nullptr != this->pbase() )
		{
			if( this->pptr() != this->pbase() )
				this->overflow( traits_type::eof() );
			memory_traits::free( this->pbase() );
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
			std::size_t avail = memory_traits::distance( this->pptr(), this->epptr() );
			std::streamsize ret;
			if(__n >  static_cast<std::streamsize>(avail) )
				ret = static_cast<std::streamsize>(avail);
			else
				ret = __n;
			std::copy(__s, ( __s + ret ), pos);
			this->pbump( ret );
			return ret;
		}
		return 0;
	}

	virtual int_type overflow(int_type __c) override
	{
		std::error_code ec;
		const uint8_t *wpos = reinterpret_cast<const uint8_t*>( this->pbase() );
		std::size_t to_write = memory_traits::distance( this->pbase(), this->pptr() ) * sizeof(char_type);
		assert( (to_write % sizeof(char_type) ) == 0 );
		std::size_t written;
		while( to_write > 0 ) {
			written = ch_->write(ec, wpos, to_write);
			if(ec)
				return traits_type::eof();
			wpos += written;
			to_write -= written;
		}
		if( traits_type::not_eof(__c) ) {
			char_type b = traits_type::to_char_type(__c);
			ch_->write(ec, reinterpret_cast<const uint8_t*>(&b), sizeof(char_type) );
			if(ec)
				return traits_type::eof();
		}
		clear();
		return 1;
	}

	virtual int sync() override
	{
		if( this->pptr() != this->pbase() )
			return this->overflow( traits_type::eof() );
		return 0;
	}

public:
	s_write_channel ch_;
	std::atomic_size_t ref_count_;

	inline friend void intrusive_ptr_add_ref(self_type* const obj) noexcept
	{
		obj->ref_count_.fetch_add(1, std::memory_order_relaxed);
	}

	inline friend void intrusive_ptr_release(self_type* const obj) noexcept
	{
		if(1 == obj->ref_count_.fetch_sub(1, std::memory_order_acquire) ) {
			std::atomic_thread_fence( std::memory_order_release);
			delete obj;
		}
	}
};


template<typename __char_type>
class channel_ostream:public std::basic_ostream<__char_type, std::char_traits<__char_type> > {
	channel_ostream(const channel_ostream&) = delete;
	channel_ostream& operator=(const channel_ostream&) = delete;
private:
	typedef std::basic_ostream<__char_type, std::char_traits<__char_type> > super_type;
	typedef ochannel_streambuf<__char_type, std::char_traits<__char_type> > streambuf_type;
	typedef boost::intrusive_ptr<streambuf_type> s_streambuf_type;
public:
	channel_ostream(s_write_channel&& ch):
		super_type(),
		sb_( new streambuf_type(
		         std::forward<s_write_channel>(ch),
		         memory_traits::page_size() )
		   )
	{
		this->init( sb_.get() );
	}

	channel_ostream(const s_write_channel& ch):
		channel_ostream( s_write_channel(ch) )
	{}

	virtual ~channel_ostream() override
	{}

private:
	s_streambuf_type sb_;
};

template<typename __char_type, class __traits_type >
class ichannel_streambuf final: public std::basic_streambuf<__char_type, __traits_type > {
private:
	typedef std::basic_streambuf<__char_type, __traits_type > super_type;
	typedef ichannel_streambuf<__char_type, __traits_type> self_type;
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

	ichannel_streambuf(s_read_channel&& src, std::size_t buffer_size) :
		super_type(),
		rch_( std::forward<s_read_channel>(src) ),
		ref_count_(0)
	{
		char_type *buff = static_cast<char_type*>( memory_traits::malloc( buffer_size * sizeof(char_type) ) );
		if(nullptr == buff ) {
			std::error_code ec = std::make_error_code(std::errc::not_enough_memory);
			ios_check_error_code( "input stream buff ", ec );
		}
		super_type::setg(buff, buff, buff + buffer_size);
	}

	virtual super_type* setbuf(char_type* data, std::streamsize size) override
	{
		return this;
	}

	virtual std::streamsize xsgetn(char_type* __s, std::streamsize __n) override
	{
		if(0 == __n)
			return 0;
		const char_type *pos = this->gptr();
		if( (pos + __n) >= this->egptr() ) {
			__n =  memory_traits::distance(pos, this->egptr());
			if(0 == __n && traits_type::not_eof( this->underflow() ) )
				return -1;
		}
		std::copy(pos, (pos+__n), __s);
		super_type::pbump(__n);
		return __n;
	}

	virtual int_type pbackfail(int_type __c) override
	{
		char_type *pos = this->gptr() - 1;
		*pos = traits_type::to_char_type( __c );
		this->pbump(-1);
		return 1;
	}

	virtual int_type underflow() override
	{
		std::size_t ret = read_more_data();
		return !ret ? traits_type::eof() : static_cast<int_type>(ret);
	}

	virtual int sync() override
	{
		return read_more_data() > 0;
	}

	virtual std::streamsize showmanyc() override
	{
		return static_cast<std::streamsize>(
					memory_traits::distance( this->gptr(), this->egptr() )
				);
	}

	virtual ~ichannel_streambuf() override
	{
		void* buff = static_cast<void*>( this->eback() );
		if(nullptr != buff )
			memory_traits::free( buff );
	}

private:

	std::size_t read_more_data()
	{
		const std::size_t count  = memory_traits::distance( this->eback(), this->egptr() );
		std::error_code ec;
		std::size_t res = rch_->read(ec, reinterpret_cast<uint8_t*>( this->eback() ), count);
		ios_check_error_code( "input stream buff ", ec );
		super_type::setg(this->eback(), this->eback(), this->egptr() );
		return res;
	}

	s_read_channel rch_;
	std::atomic_size_t ref_count_;

	inline friend void intrusive_ptr_add_ref(self_type* const obj) noexcept
	{
		obj->ref_count_.fetch_add(1, std::memory_order_relaxed);
	}

	inline friend void intrusive_ptr_release(self_type* const obj) noexcept
	{
		if(1 == obj->ref_count_.fetch_sub(1, std::memory_order_acquire) ) {
			std::atomic_thread_fence( std::memory_order_release);
			delete obj;
		}
	}
};

template<typename _char_type>
class channel_istream final:public std::basic_istream<_char_type, std::char_traits<_char_type> >
{
	channel_istream(const channel_istream&) = delete;
	channel_istream& operator=(const channel_istream&) = delete;
private:
	typedef std::basic_istream<_char_type, std::char_traits<_char_type> > super_type;
	typedef ichannel_streambuf<_char_type, std::char_traits<_char_type> > streambuf_type;
	typedef boost::intrusive_ptr<streambuf_type> s_streambuf_type;
public:
	typedef _char_type  char_type;
	typedef typename super_type::int_type int_type;
	typedef typename super_type::pos_type pos_type;
	typedef typename super_type::off_type off_type;
	typedef typename super_type::traits_type traits_type;

	channel_istream(s_read_channel&& src, std::size_t buffer_size):
		super_type(),
		sb_( new streambuf_type(
					std::forward<s_read_channel>(src),
					buffer_size
				 ), true
			)
	{
		assert( sb_ );
		this->init( sb_.get() );
	}

	channel_istream(s_read_channel&& src):
		channel_istream(
			std::forward<s_read_channel>(src),
			memory_traits::page_size()
		)
	{}

	channel_istream(const s_read_channel& src):
		channel_istream( s_read_channel(src) )
	{}

	inline void swap(channel_istream& rhs) noexcept
	{
		sb_.swap( rhs );
		super_type::swap( rhs );
	}

	channel_istream(channel_istream&& other) noexcept:
		super_type( std::forward<channel_istream>(other) ),
		sb_( std::move( other.sb_ ) )
	{}

	channel_istream& operator=(channel_istream&& rhs) noexcept
	{
		channel_istream( std::forward<channel_istream>(rhs) ).swap( *this );
		return *this;
	}

	virtual ~channel_istream() override
	{}

private:
	s_streambuf_type sb_;
};

} // namespace io

#endif // __IO_STREAMS_HPP_INCLUDED__
