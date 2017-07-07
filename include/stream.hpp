#ifndef __IO_STREAMS_HPP_INCLUDED__
#define __IO_STREAMS_HPP_INCLUDED__

#include "config.hpp"
#include "channels.hpp"

#include <sstream>

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

namespace io {

namespace detail {

template< typename __char_t, class __traits>
class w_channel_stream_buff final:
	public std::basic_stringbuf< __char_t, __traits, io::h_allocator<__char_t> >
{
	w_channel_stream_buff(const w_channel_stream_buff&) = delete;
	w_channel_stream_buff& operator=(const w_channel_stream_buff&) = delete;
public:
	typedef __char_t char_type;
	typedef __traits traits_type;
	typedef typename traits_type::int_type int_type;
	typedef typename traits_type::pos_type pos_type;
	typedef typename traits_type::off_type off_type;

	typedef std::basic_streambuf<char_type, traits_type> stream_buff_type;
	typedef std::basic_stringbuf<char_type, traits_type, io::h_allocator<char_type> > base_type;
	typedef std::basic_ios<char_type, traits_type> ios_type;
	friend class std::ios_base; // For sync_with_stdio.

private:
	typedef typename base_type::__string_type string_type;
public:

	w_channel_stream_buff():
		base_type( string_type(1024) , std::ios_base::out)
	{}

	w_channel_stream_buff(w_channel_stream_buff&& oth):
		base_type( std::forward<base_type>(oth) ),
		out_( std::forward<s_write_channel>(oth.out_) )
	{}

	w_channel_stream_buff& operator=(w_channel_stream_buff&& oth) noexcept
	{
		w_channel_stream_buff( std::forward<w_channel_stream_buff>(oth) ).swap( *this );
		return *this;
	}

	w_channel_stream_buff(const s_write_channel& out) noexcept:
		base_type(std::ios_base::out),
		out_(out)
	{}

	virtual ~w_channel_stream_buff() noexcept override
	{}

	void swap(w_channel_stream_buff& sb) {
		base_type::swap( reinterpret_cast<base_type>(sb) );
		out_.swap( sb.out_ );
	}

	virtual int sync() override {
		string_type str( base_type::str() );
		str.push_back( static_cast<char_type>(0) );
		const char_type* s = str.data();
		std::size_t len = traits_type::length(s);
		std::error_code ec;
		if ( out_->write(ec, reinterpret_cast<const uint8_t*>( s ), len * sizeof(char_type) ) == 0 ) {
			len = 0;
		}
		if(ec) {
			#ifndef IO_NO_EXCEPTIONS
				throw std::system_error(ec);
			#else
				std::fprintf( stderr, ec.message().data() );
				std::unexpected();
			#endif // IO_NO_EXCEPTIONS
		}
		// clear
		char_type clear_d[1] = {char_type('\0')};
		base_type::setbuf(&clear_d[0], 1);
		return len;
	}

private:
	s_write_channel out_;
};

template< typename __char_t, class __traits>
class r_channel_streambuff final:public std::basic_stringbuf<__char_t, __traits, io::h_allocator<__char_t> > {
	r_channel_streambuff(const r_channel_streambuff&) = delete;
	r_channel_streambuff& operator=(const r_channel_streambuff&) = delete;
private:
	typedef std::basic_stringbuf<__char_t, __traits, io::h_allocator<__char_t> > base_type;
	typedef typename base_type::__string_type string_type;
public:

	typedef __char_t char_type;
	typedef __traits traits_type;
	typedef typename traits_type::int_type int_type;
	typedef typename traits_type::pos_type pos_type;
	typedef typename traits_type::off_type off_type;

	typedef std::basic_streambuf<char_type, traits_type> stream_buff_type;
	typedef std::basic_ios<char_type, traits_type> ios_type;
	friend class std::ios_base; // For sync_with_stdio.

	r_channel_streambuff(const s_read_channel& in):
		base_type(std::ios_base::in),
		in_(in)
	{}

	virtual ~r_channel_streambuff() override
	{}

	r_channel_streambuff(r_channel_streambuff&& oth):
		base_type( std::forward<base_type>(oth) ),
		in_( std::forward<s_read_channel>(oth.in_) )
	{}

	r_channel_streambuff& operator=(r_channel_streambuff&& oth) noexcept
	{
		r_channel_streambuff tmp( std::forward<r_channel_streambuff>(oth) );
		swap( tmp );
		return *this;
	}

	void swap(r_channel_streambuff& sb) {
		base_type::swap( reinterpret_cast<base_type>(sb) );
		in_.swap( sb.in_ );
	}

	virtual int_type underflow() override
	{
		std::error_code ec;
		uint8_t buff[1024];
		std::size_t result = in_->read(ec, buff , 1024 );
		if(ec) {
		#ifndef IO_NO_EXCEPTIONS
			throw std::system_error(ec);
		#else
			std::fprintf( stderr, ec.message().data() );
			std::unexpected();
		#endif // IO_NO_EXCEPTION
		}
		if(result > 0) {
			base_type::setbuf(reinterpret_cast<char_type*>(buff), result / sizeof(char_type) );
			return base_type::underflow();
		}
		return traits_type::eof();
	}

private:
	s_read_channel in_;
};

} // namespace detail

/// \brief Buffered output stream wrapper on write channel
template< typename __char_t, class __traits = std::char_traits<__char_t> >
class channel_ostream final:public std::basic_ostream<__char_t, __traits>
{
	typedef std::basic_ostream<__char_t, __traits> base_type;
	channel_ostream(const channel_ostream&) = delete;
	channel_ostream& operator=(const channel_ostream&) = delete;
public:
	typedef detail::w_channel_stream_buff<__char_t, __traits> __streambuf_type;
	channel_ostream(const s_write_channel& out):
		base_type(),
		buff_(out)
	{
		this->init( const_cast<__streambuf_type*>(&buff_) );
	}
	virtual ~channel_ostream() override
	{}
private:
	__streambuf_type buff_;
};

/// \brief Buffered input stream wrapper on read channel
template< typename __char_t, class __traits = std::char_traits<__char_t> >
class channel_istream final:public std::basic_istream<__char_t, __traits>
{
	typedef std::basic_istream<__char_t, __traits> base_type;
	channel_istream(const channel_istream& ) = delete;
	channel_istream& operator=(const channel_istream&) = delete;
public:
	typedef detail::r_channel_streambuff<__char_t, __traits> __streambuf_type;
	channel_istream(const s_read_channel& in):
		base_type(),
		buff_(in)
	{
		this->init( const_cast<__streambuf_type*>(&buff_) );
	}
	virtual ~channel_istream() override
	{}
private:
	__streambuf_type buff_;
};

} // namespace io

#endif // __IO_STREAMS_HPP_INCLUDED__
