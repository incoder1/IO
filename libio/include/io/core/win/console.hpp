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
#ifndef _IO_WIN_CONSOLE_HPP_INCLUDED__
#define _IO_WIN_CONSOLE_HPP_INCLUDED__

#include <io/config/libio_config.hpp>

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <atomic>
#include <memory>

#include <wincon.h>
#include <tchar.h>

#include "criticalsection.hpp"
#include "errorcheck.hpp"

#include "io/textapi/nio.hpp"
#include "io/textapi/stream.hpp"

#ifndef _CU
#	define _CU(quote) L##quote
#endif // __LOCALE_TEXT

namespace io {

class console;

namespace win {


class IO_PUBLIC_SYMBOL console_channel final: public read_write_channel {
	console_channel(const console_channel&) = delete;
	console_channel& operator=(const console_channel&) = delete;
	console_channel(console_channel&&) = delete;
	console_channel& operator=(console_channel&&) = delete;
public:
	console_channel(::HANDLE hcons) noexcept;
	virtual ~console_channel() noexcept override;
	virtual std::size_t read(std::error_code& err,uint8_t* const buff, std::size_t bytes) const noexcept override;
	virtual std::size_t write(std::error_code& err, const uint8_t* buff,std::size_t size) const noexcept override;
private:
	::HANDLE hcons_;
};


s_write_channel IO_PUBLIC_SYMBOL conv_write_channel(const s_write_channel& ch);
s_read_channel IO_PUBLIC_SYMBOL conv_read_channel(const s_read_channel& ch);

} // namesapce win

enum class text_color: uint8_t {
	navy_blue	 = 0x01,
	navy_green   = 0x02,
	navy_aqua	 = 0x03,
	navy_red	 = 0x04,
	magenta		 = 0x05,
	brown		 = 0x06,
	white		 = 0x07,
	gray		 = 0x08,
	light_blue   = 0x09,
	light_green  = 0x0A,
	light_aqua   = 0x0B,
	light_red	 = 0x0C,
	light_purple = 0x0D,
	yellow		 = 0x0E,
	bright_white = 0x0F
};

/// \brief System console (terminal window) low level access API.
/// Windows based implementation based on
/// <wincon.h> console API with UNICODE only input/output.
/// Console is usefull to implement a software like POSIX 'cat','more' etc.
/// Console is UNICODE only! I.e. supports multi-languages text at time
/// Non whide characters expected to be in UTF-8 (or compatiable) code page
class IO_PUBLIC_SYMBOL console {
	console(const console&) = delete;
	console operator=(const console&) = delete;
public:
	typedef ::HANDLE native_id;
	typedef channel_ostream<char> cons_ostream;
	typedef channel_istream<char> cons_istream;
private:

	friend class console_input_stream;
	friend class console_output_stream;
	friend class console_error_stream;
	friend class console_out_writer;

	static ::HANDLE in_handle() noexcept;
	static ::HANDLE out_handle() noexcept;
	static ::HANDLE err_handle() noexcept;

	::WORD	current_stream_attributes(::HANDLE hcons) noexcept;

public:
	/// Opens or connect to existing Windows console
	/// console will be switched to UTF_16LE code page if not yet
	/// WARN! Standard stream redirections, i.e. pipes like my.exe >> ret.log is not going to work unless
	/// console holding strams
	console();

	/// Releases allocated console, and returns original console codepage
	~console() noexcept;

	/// Change default console output colors
	/// \param in color for input stream
	/// \param out color for output stream
	/// \param err color for error stream
	void change_colors(const text_color in,const text_color out,const text_color err) noexcept;

	/// Change input stream console output color
	/// \param clr new color
	void change_in_color(const text_color clr) noexcept;


	/// Change stdout stream console output color
	/// \param clr new color
	void change_out_color(const text_color clr) noexcept;


	/// Change stderr stream console output color
	/// \param clr new color
	void change_err_color(const text_color crl) noexcept;

	void reset_in_color() noexcept;
	void reset_out_color() noexcept;
	void reset_err_color() noexcept;

private:
	s_read_channel in_;
	s_write_channel out_;
	s_write_channel err_;
	::UINT prev_charset_;
	::WORD default_in_attr_;
	::WORD default_out_attr_;
	::WORD default_err_attr_;
	bool need_release_;
};

class console_out_writer: public writer {
public:
	explicit console_out_writer(console& cons):
		writer( win::conv_write_channel(cons.out_) )
	{}
};


class console_input_stream: public cnl_istream {
public:
	explicit console_input_stream(console& cons):
		cnl_istream( win::conv_read_channel( cons.in_ ) )
	{}
};


class base_console_out_stream: public cnl_ostream {
public:
	base_console_out_stream(const s_write_channel& fb):
		cnl_ostream( win::conv_write_channel(fb) )
	{}
	virtual void change_color(text_color clr) const = 0;
	virtual void reset_color() const = 0;
};


} // namesapce io

#endif // _IO_WIN_CONSOLE_HPP_INCLUDED__
