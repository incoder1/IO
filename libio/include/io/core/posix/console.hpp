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
#ifndef __IO_POSIX_CONSOLE_HPP_INCLUDED__
#define __IO_POSIX_CONSOLE_HPP_INCLUDED__

#include <io/config/libio_config.hpp>

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <io/core/channels.hpp>
#include <io/textapi/nio.hpp>
#include <io/textapi/stream.hpp>

namespace io
{

enum class text_color
{
	navy_blue,
	navy_green,
	navy_aqua,
	navy_red,
	magenta,
	brown,
	white,
	gray,
	light_blue,
	light_green,
	light_aqua,
	light_red,
	light_purple,
	yellow,
	bright_white
};

namespace posix
{

class IO_PUBLIC_SYMBOL console_channel final: public io::read_write_channel
{
	console_channel(const console_channel&) = delete;
	console_channel& operator=(const console_channel&) = delete;
public:
	console_channel(const os_descriptor_t stream) noexcept;
	virtual ~console_channel() = default;
	virtual std::size_t read(std::error_code& ec,uint8_t* const buff, std::size_t bytes) const noexcept override;
	virtual std::size_t write(std::error_code& ec, const uint8_t* buff,std::size_t size) const noexcept override;
	void change_color(text_color attr) noexcept;
private:
	os_descriptor_t stream_;
};

DECLARE_IPTR(console_channel);

} // namespace posix

class IO_PUBLIC_SYMBOL console
{
private:
	friend class console_input_stream;
	friend class console_output_stream;
	friend class console_error_stream;

	friend class console_out_writer;
public:
	console();

	/// Changes default console output colors
	/// \param in color for input stream
	/// \param out color for output stream
	/// \param err color for error stream
	void change_colors(const text_color in,const text_color out,const text_color err) noexcept;

	/// Changes input stream console output color
	/// \param clr new color
	void change_in_color(const text_color clr) noexcept;

	/// Changes stdout stream console output color
	/// \param clr new color
	void change_out_color(const text_color clr) noexcept;

	/// Changes stderr stream console output color
	/// \param clr new color
	static void change_err_color(const text_color crl) noexcept;

	void reset_in_color() noexcept;

	void reset_out_color() noexcept;

	void reset_err_color() noexcept;


private:
	posix::s_console_channel in_;
	posix::s_console_channel out_;
	posix::s_console_channel err_;
};

class console_out_writer: public writer {
public:
	explicit console_out_writer(console& cons):
		writer( cons.out_ )
	{}
};

class console_input_stream: public cnl_istream {
public:
	explicit console_input_stream(console& cons):
		cnl_istream(  cons.in_  )
	{}
};

class base_console_out_stream: public cnl_ostream {
public:
	base_console_out_stream(const s_write_channel& fb):
		cnl_ostream( fb )
	{}
	virtual void change_color(text_color clr) const = 0;
	virtual void reset_color() const = 0;
};


} // namespace io

#endif // __IO_POSIX_CONSOLE_HPP_INCLUDED__
