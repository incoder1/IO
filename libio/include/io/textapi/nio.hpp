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

namespace detail {

#ifdef IO_HAS_CONNCEPTS

template<typename C>
requires is_charater_v<C>
struct endl
{};

#else

template<typename C>
struct endl
{};

#endif // IO_HAS_CONNCEPTS

#ifdef __IO_WINDOWS_BACKEND__

template<>
struct endl<char> {
	static constexpr const char* symbol = "\n\r";
};

#if defined(__HAS_CPP_17) && defined(__cpp_char8_t)
template<>
struct endl<char8_t> {
	static constexpr const char8_t* symbol = u8"\n\r";
};
#endif // defined

template<>
struct endl<wchar_t> {
	static constexpr const wchar_t* symbol = L"\n\r";
};

template<>
struct endl<char16_t> {
	static constexpr const char16_t* symbol = u"\n\r";
};

template<>
struct endl<char32_t> {
	static constexpr const char32_t* symbol = U"\n\r";
};

#else

template<>
struct endl<char> {
	static constexpr const char* symbol = "\n";
};

#if defined(__HAS_CPP_17) && defined(__cpp_char8_t)
template<>
struct endl<char8_t> {
	static constexpr const char8_t* symbol = u8"\n";
};
#endif // defined

template<>
struct endl<wchar_t> {
	static constexpr const wchar_t* symbol = L"\n";
};

template<>
struct endl<char16_t> {
	static constexpr const char16_t* symbol = u"\n";
};

template<>
struct endl<char32_t> {
	static constexpr const char32_t* symbol = U"\n";
};

#endif // __IO_WINDOWS_BACKEND__

} // namespace detail

class pump;
DECLARE_IPTR(pump);

class pump : public object {
	pump(pump&) = delete;
	pump& operator=(pump&) = delete;
protected:
	pump() noexcept;
public:
	virtual std::size_t pull(std::error_code& ec, uint8_t* const to, std::size_t bytes) noexcept = 0;
	virtual bool sync(std::error_code& ec) noexcept;
};

class channel_pump: public pump {
protected:
	explicit channel_pump(s_read_channel&& src) noexcept;
public:
	static s_pump create(std::error_code& ec, s_read_channel&& src) noexcept;
	virtual std::size_t pull(std::error_code& ec, uint8_t* const to, std::size_t bytes) noexcept override;
private:
	s_read_channel src_;
};

class buffered_channel_pump: public channel_pump {
protected:
	buffered_channel_pump(s_read_channel&& src,byte_buffer&& buff) noexcept;
public:
	static s_pump create(std::error_code& ec, s_read_channel&& src, std::size_t buffer_size) noexcept;
	virtual std::size_t pull(std::error_code& ec, uint8_t* const to, std::size_t bytes) noexcept override;
	virtual bool sync(std::error_code& ec) noexcept override;
protected:
	byte_buffer read_buff_;
};

class charset_converting_channel_pump final: public buffered_channel_pump {
private:
	charset_converting_channel_pump(s_read_channel&& src, byte_buffer&& rb, byte_buffer&& cvb,s_charset_converter&& cvt) noexcept;
public:
	static s_pump create(std::error_code& ec, s_read_channel&& src, const charset* from, const charset* to, std::size_t buffer_size) noexcept;
	virtual std::size_t pull(std::error_code& ec, uint8_t* const to, std::size_t bytes) noexcept override;
	bool sync(std::error_code& ec) noexcept;
private:
	byte_buffer cvt_buff_;
	s_charset_converter cvt_;
};

class funnel;
DECLARE_IPTR(funnel);

class funnel: public object {
protected:
	funnel() noexcept;
public:
	virtual std::size_t push(std::error_code& ec, const uint8_t* src, std::size_t bytes) noexcept = 0;
	virtual void flush(std::error_code& ec) noexcept;
};

class channel_funnel: public funnel
{
protected:
	channel_funnel(s_write_channel&& dst) noexcept;
public:
	static s_funnel create(std::error_code& ec, s_write_channel&& dst) noexcept;
	virtual std::size_t push(std::error_code& ec, const uint8_t* src, std::size_t bytes) noexcept override;
private:
	s_write_channel dst_;
};

class buffered_channel_funnel: public channel_funnel {
protected:
	buffered_channel_funnel(s_write_channel&& dst, byte_buffer&& buff) noexcept;
public:
	static s_funnel create(std::error_code& ec, s_write_channel&& dst, std::size_t buffer_size) noexcept;
	virtual std::size_t push(std::error_code& ec, const uint8_t* src, std::size_t bytes) noexcept override;
	virtual void flush(std::error_code& ec) noexcept override;
protected:
	byte_buffer write_buff_;
};

class charset_converting_channel_funnel final: public buffered_channel_funnel {
private:
	charset_converting_channel_funnel(s_write_channel&& dst, byte_buffer&& buff,s_charset_converter&& cvt) noexcept;
public:
	static s_funnel create(std::error_code& ec, s_write_channel&& dst, const charset* from, const charset* to, std::size_t buffer_size) noexcept;
	virtual void flush(std::error_code& ec) noexcept override;
private:
	s_charset_converter cvt_;
};

#ifndef IO_HAS_CONNCEPTS
template<typename C, class ___type_restriction = void>
class basic_reader;
template<typename C, class ___type_restriction = void>
class basic_writer;
#endif // IO_HAS_CONNCEPTS

#ifdef IO_HAS_CONNCEPTS
template<typename C>
	requires( is_charater_v<C> )
class basic_reader
#else
template<typename C>
class basic_reader<C,
				typename std::enable_if<
					is_charater< C >::value >::type
				>
#endif // IO_HAS_CONNCEPTS
{
	basic_reader(const basic_reader&) = delete;
	basic_reader& operator=(const basic_reader&) = delete;
public:

	typedef C char_type;
	typedef std::char_traits<char_type> char_traits;
	static constexpr std::size_t char_width = sizeof(char_type);
	typedef typename std::basic_string<char_type> string;

	explicit basic_reader(s_pump&& src) noexcept:
		ec_(),
		src_( src )
	{}

	std::size_t read(std::error_code& ec, char_type* const to, std::size_t chars) noexcept
	{
		std::size_t bytes_read = src_->pull( ec, reinterpret_cast<char_type*>(to), chars * sizeof(char_type) );
		return bytes_read / sizeof(char_type);
	}

	std::size_t read(char_type* const to, std::size_t chars)
	{
		std::size_t ret = read(ec_, to, chars);
		io::check_error_code(ec_);
		return ret;
	}

private:
	std::error_code ec_;
	s_pump src_;
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
	/// \brief a destination data funnel
	/// \brief internal character buffer size
	basic_writer(const s_funnel& dst,std::size_t buffer_size) noexcept:
		ec_(),
		buffer_(io::byte_buffer::allocate(ec_, buffer_size)),
		dst_(dst)
	{}

	/// Construct new writer on top of the write channel with default internal character buffer
	/// \brief a destination output channgel
	explicit basic_writer(const s_funnel& dst) noexcept:
		basic_writer(dst, memory_traits::page_size())
	{}

	basic_writer(basic_writer&& other) noexcept:
		ec_( std::exchange(other.ec_, std::error_code() ) ),
		buffer_( std::exchange(other.buffer_, byte_buffer()) ),
		dst_( std::exchange(other.dst_, s_funnel()) )
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
			dst_->push(ec_, buffer_.position().get(), buffer_.length());
			dst_->flush(ec_);
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

	void writeln(const C* str) noexcept
	{
		write(str, char_traits::length(str) );
		write(detail::endl<C>::symbol);
	}

	void write(const std::basic_string<C>& str ) noexcept
	{
		write( str.data(), str.size() );
	}

	void writeln(const std::basic_string<C>& str ) noexcept
	{
		write( str.data(), str.size() );
		write(detail::endl<C>::symbol);
	}

	void write(const const_string& str) noexcept
	{
		write( str.data(), str.size() );
	}

	void writeln(const const_string& str) noexcept
	{
		write( str.data(), str.size() );
		write(detail::endl<C>::symbol);
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
			dst_->push(ec_,  buffer_.position().get(), buffer_.length() );
			//transmit_buffer(ec_, dst_, buffer_.position().get(), buffer_.length() );
			if(!ec_)
				buffer_.clear();
		}
	}
private:
	std::error_code ec_;
	byte_buffer buffer_;
	s_funnel dst_;
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
