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
#ifndef __IO_NIO_HPP_INCLUDED__
#define __IO_NIO_HPP_INCLUDED__

#include <io/config/libio_config.hpp>

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <atomic>
#include <exception>
#include <string>
#ifdef __HAS_CPP_17
#	include <string_view>
#endif // __HAS_CPP_17

#include <io/core/channels.hpp>
#include <io/core/conststring.hpp>
#include <io/core/error_check.hpp>
#include <io/core/type_traits_ext.hpp>

#include <io/textapi/charset_converter.hpp>

#ifdef __IO_WINDOWS_BACKEND__
typedef wchar_t unicode_char;
#endif // __IO_WINDOWS_BACKEND__

#ifdef __IO_POSIX_BACKEND__
typedef char32_t unicode_char;
#endif // __IO_WINDOWS_BACKEND__

namespace io {


#ifndef IO_HAS_CONNCEPTS
template<typename C, class ___type_restriction = void>
class basic_reader;
template<typename C, class ___type_restriction = void>
class basic_writer;
#endif // IO_HAS_CONNCEPTS


#ifdef IO_HAS_CONNCEPTS
template<typename C>
requires is_charater_v<C>
class basic_reader {
#else
template<typename C>
class basic_reader<C, typename std::enable_if< is_charater< C >::value >::type > {
#endif // IO_HAS_CONNCEPTS
	basic_reader(const basic_reader&) = delete;
	basic_reader& operator=(const basic_reader&) = delete;
public:
	typedef C char_type;
	typedef std::char_traits<char_type> char_traits;
	static constexpr std::size_t char_width = sizeof(char_type);
	typedef typename std::basic_string<char_type> string;

private:

	inline char_type* to_char_ptr(const uint8_t* px) noexcept
	{
		return reinterpret_cast<char_type*>( const_cast<uint8_t*>(px) );
	}

	inline char_type* pos() noexcept
	{
		return to_char_ptr( rb_.position() );
	}

	inline char_type* last() noexcept
	{
		return to_char_ptr( rb_.last() );
	}

	inline std::size_t length_to_bytes(const std::size_t length) noexcept
	{
		return length * char_width;
	}

	std::size_t underflow(std::error_code& ec) noexcept
	{
		std::size_t result = 0;
		if( io_likely(rb_) ) {
			result = src_->read(ec, const_cast<uint8_t*>(rb_.position().get()), rb_.capacity() );
			if( !ec ) {
				rb_.move(result);
				rb_.flip();
			}
		}
		else {
			// handle out of memory when constructing a reader
			ec = std::make_error_code(std::errc::not_enough_memory);
		}
		return length_to_bytes(result);
	}

	std::size_t buffered() noexcept
	{
		return length_to_bytes( rb_.length() );
	}

public:

	basic_reader(basic_reader&&) noexcept = default;
	basic_reader& operator=(basic_reader&&) noexcept = default;

	explicit basic_reader(const s_read_channel& src, std::size_t buff_size = 1024 ) noexcept:
		src_( src ),
		rb_()
	{
		std::error_code ec;
		rb_ = byte_buffer::allocate(ec, buff_size);
	}


	std::size_t read(std::error_code& ec, char_type* const to, std::size_t chars) noexcept
	{
		std::size_t result = 0;
		std::size_t available = rb_.empty() ? underflow(ec) : buffered();
		if(available > 0 && !ec) {
			const char_type* px = reinterpret_cast<const char_type*>( rb_.position().get() );
			if( available > chars ) {
				char_traits::copy(to, px, result);
				result = chars;
				rb_.move( result * char_width );
			}
			else {
				result = available;
				char_traits::copy(to, px, result);
				rb_.clear();
			}
		}
		return result;
	}

private:
	s_read_channel src_;
	byte_buffer rb_;
};

/// !\brief Character output interface
#ifdef IO_HAS_CONNCEPTS
template<typename C>
requires is_charater_v<C>
class basic_writer {
#else
template<typename C>
class basic_writer<C, typename std::enable_if< is_charater< C >::value >::type > {
#endif // IO_HAS_CONNCEPTS
//requires is_charater_v<C> class basic_writer {
	basic_writer(const basic_writer&) = delete;
	basic_writer operator=(const basic_writer&) = delete;
public:

	typedef C char_type;
	typedef std::char_traits<char_type> char_traits;
	static constexpr std::size_t char_width = sizeof(char_type);
	typedef typename std::basic_string<char_type> string;

private:

	inline std::size_t length_to_bytes(const std::size_t length) noexcept
	{
		return length * char_width;
	}

public:
	/// Construct new writer on top of the write channel
	/// \brief a destination output channgel
	/// \brief internal character buffer size
	basic_writer(const s_write_channel& dst,std::size_t buffer_size) noexcept:
		ec_(),
		buffer_(io::byte_buffer::allocate(ec_, buffer_size)),
		dst_(dst)
	{}

	/// Construct new writer on top of the write channel with default internal character buffer
	/// \brief a destination output channgel
	explicit basic_writer(const s_write_channel& dst) noexcept:
		basic_writer(dst, memory_traits::page_size())
	{}

	basic_writer(basic_writer&& other) noexcept:
		ec_( std::exchange(other.ec_, std::error_code() ) ),
		buffer_( std::exchange(other.buffer_, byte_buffer()) ),
		dst_( std::exchange(other.dst_, s_write_channel()) )
	{}

	basic_writer& operator=(basic_writer&& other) noexcept
	{
		basic_writer( other ).swap( *this );
		return *this;
	}

	~basic_writer() noexcept
	{
		if(dst_ && !buffer_.empty() && !ec_ ) {
			buffer_.flip();
			transmit_buffer(ec_, dst_, buffer_.position().get(), buffer_.length() );
		}
	}

	void write(C ch) noexcept
	{
		if( buffer_.full() )
			flush();
		buffer_.put(ch);
	}

	void write(const C* str, std::size_t len) noexcept
	{
		if( buffer_.available() < length_to_bytes(len) )
			flush();
		buffer_.put(str, len);
	}

	void write(const C* str) noexcept
	{
		return write(str, char_traits::length(str) );
	}

	void write(const std::basic_string<C>& str ) noexcept
	{
		return write( str.data(), str.size() );
	}

	void write(const const_string& str) noexcept
	{
		return write( str.data(), str.size() );
	}

#ifdef __HAS_CPP_17
	void write(const std::string_view& str) noexcept
	{
		return write( str.data(), str.size() );
	}
#endif // __HAS_CPP_17

	explicit operator bool() const noexcept
	{
		return 0 == ec_.value();
	}

	std::error_code last_error() const noexcept
	{
		return ec_;
	}

	void swap(basic_writer& other) noexcept
	{
		dst_.swap( other.dst_ );
		buffer_.swap(other.buffer_);
		std::swap(ec_, other.ec_);
	}

	void flush() noexcept
	{
		if(!buffer_.empty() && !ec_) {
			buffer_.flip();
			transmit_buffer(ec_, dst_, buffer_.position().get(), buffer_.length() );
			if(!ec_)
				buffer_.clear();
		}
	}
private:
	std::error_code ec_;
	byte_buffer buffer_;
	s_write_channel dst_;
};

typedef basic_reader<char> reader;
typedef basic_reader<wchar_t> wreader;
typedef basic_reader<char16_t> u16_reader;
typedef basic_reader<char32_t> u32_reader;
typedef basic_writer<char> writer;
typedef basic_writer<wchar_t> wwriter;
typedef basic_writer<char16_t> u16_writer;
typedef basic_writer<char32_t> u32_writer;
#ifdef IO_HAS_CHAR8_T
typedef basic_writer<char8_t> u8_writer;
typedef basic_reader<char8_t> u8_reader;
#endif // IO_HAS_CHAR8_T

} // namespace io

#endif // __IO_TEXT_HPP_INCLUDED__
