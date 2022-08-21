/*
 *
 * Copyright (c) 2016-2020
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef _IO_WIN_CONSOLE_HPP_INCLUDED__
#define _IO_WIN_CONSOLE_HPP_INCLUDED__

#include <config.hpp>

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE


#include <atomic>
#include <memory>

#include <text.hpp>
#include <stream.hpp>

#include <wincon.h>
#include <tchar.h>

#include "criticalsection.hpp"
#include "errorcheck.hpp"

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
	navy_blue	= 0x01,
	navy_green   = 0x02,
	navy_aqua	= 0x03,
	navy_red	 = 0x04,
	magenta	  = 0x05,
	brown		= 0x06,
	white		= 0x07,
	gray		 = 0x08,
	light_blue   = 0x09,
	light_green  = 0x0A,
	light_aqua   = 0x0B,
	light_red	= 0x0C,
	light_purple = 0x0D,
	yellow	   = 0x0E,
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

	/// Returns console character set, always UTF-16LE for Windows
	/// \return current console character set
	/// \throw never throws
	static inline const charset& charset() noexcept
	{
		return code_pages::UTF_16LE;
	}

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

class console_output_stream final: public base_console_out_stream {

	console_output_stream(const console_output_stream&) = delete;
	console_output_stream& operator=(const console_output_stream&) = delete;

	console_output_stream(console_output_stream&&) = delete;
	console_output_stream& operator=(console_output_stream&&) = delete;

public:

	explicit console_output_stream(console& cons):
		base_console_out_stream(cons.out_),
		cons_(cons)
	{}

	virtual ~console_output_stream() noexcept override = default;

	virtual void change_color(text_color clr) const override
	{
		cons_.change_out_color(clr);
	}

	virtual void reset_color() const override
	{
		cons_.reset_out_color();
	}

private:
	console& cons_;
};

class console_error_stream: public base_console_out_stream {
	console_error_stream(const console_error_stream&) = delete;
	console_error_stream& operator=(const console_error_stream&) = delete;

	console_error_stream(console_error_stream&&) = delete;
	console_error_stream& operator=(console_error_stream&&) = delete;

public:

	explicit console_error_stream(console& cons):
		base_console_out_stream(cons.err_ ),
		cons_(cons)
	{}

	virtual ~console_error_stream() noexcept override = default;

	virtual void change_color(text_color clr) const override
	{
		cons_.change_err_color(clr);
	}

	virtual void reset_color() const override
	{
		cons_.reset_err_color();
	}
private:
	console& cons_;
};


namespace cclr {

inline void change_clr_trick(std::ostream& to, text_color tc)
{
	to.flush();
	auto *strm = reinterpret_cast<const base_console_out_stream*>( std::addressof(to) );
	strm->change_color(tc);
}

inline std::ostream& navy_blue(std::ostream& to)
{
	change_clr_trick(to,text_color::navy_blue);
	return to;
}

inline std::ostream& navy_green(std::ostream& to)
{
	change_clr_trick(to,text_color::navy_green);
	return to;
}

inline std::ostream& navy_aqua(std::ostream& to)
{
	change_clr_trick(to,text_color::navy_aqua);
	return to;
}

inline std::ostream& navy_red(std::ostream& to)
{
	change_clr_trick(to,text_color::navy_red);
	return to;
}

inline std::ostream& magenta(std::ostream& to)
{
	change_clr_trick(to,text_color::magenta);
	return to;
}

inline std::ostream& brown(std::ostream& to)
{
	change_clr_trick(to,text_color::brown);
	return to;
}

inline std::ostream& white(std::ostream& to)
{
	change_clr_trick(to,text_color::white);
	return to;
}


inline std::ostream& gray(std::ostream& to)
{
	change_clr_trick(to,text_color::gray);
	return to;
}

inline std::ostream& light_blue(std::ostream& to)
{
	change_clr_trick(to,text_color::light_blue);
	return to;
}

inline std::ostream& light_green(std::ostream& to)
{
	change_clr_trick(to,text_color::light_green);
	return to;
}

inline std::ostream& light_aqua(std::ostream& to)
{
	change_clr_trick(to,text_color::light_aqua);
	return to;
}

inline std::ostream& light_red(std::ostream& to)
{
	change_clr_trick(to,text_color::light_red);
	return to;
}

inline std::ostream& light_purple(std::ostream& to)
{
	change_clr_trick(to,text_color::light_purple);
	return to;
}

inline std::ostream& yellow(std::ostream& to)
{
	change_clr_trick(to,text_color::yellow);
	return to;
}

inline std::ostream& bright_white(std::ostream& to)
{
	change_clr_trick(to,text_color::bright_white);
	return to;
}

inline std::ostream& reset(std::ostream& to)
{
	to.flush();
	auto *strm = reinterpret_cast<const base_console_out_stream*>( std::addressof(to) );
	strm->reset_color();
	return to;
}

} // namespace cclr

} // namesapce io

#endif // _IO_WIN_CONSOLE_HPP_INCLUDED__
