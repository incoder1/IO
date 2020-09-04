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

#include <istream>
#include <ostream>

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

inline ::WORD prev_attr(::HANDLE hcons) noexcept
{
	::CONSOLE_SCREEN_BUFFER_INFO info;
	::GetConsoleScreenBufferInfo(hcons,&info);
	return info.wAttributes;
}

class IO_PUBLIC_SYMBOL console_channel final: public read_write_channel {
	console_channel(const console_channel&) = delete;
	console_channel& operator=(const console_channel&) = delete;
	console_channel(console_channel&&) = delete;
	console_channel& operator=(console_channel&&) = delete;
public:
	console_channel(::HANDLE hcons, ::WORD orig, ::WORD attr) noexcept;
	virtual ~console_channel() noexcept override;
	virtual std::size_t read(std::error_code& err,uint8_t* const buff, std::size_t bytes) const noexcept override;
	virtual std::size_t write(std::error_code& err, const uint8_t* buff,std::size_t size) const noexcept override;
private:
	friend class io::console;

	void change_color(::WORD attr) noexcept;

	::HANDLE hcons_;
	::WORD orig_attr_;
	::WORD attr_;
};

} // namesapce win

enum class text_color: uint8_t {
	navy_blue    = 0x01,
	navy_green   = 0x02,
	navy_aqua    = 0x03,
	navy_red     = 0x04,
	magenta      = 0x05,
	brown        = 0x06,
	white        = 0x07,
	gray         = 0x08,
	light_blue   = 0x09,
	light_green  = 0x0A,
	light_aqua   = 0x0B,
	light_red    = 0x0C,
	light_purple = 0x0D,
	yellow       = 0x0E,
	bright_white = 0x0F
};

inline std::ostream& operator<<(std::ostream& to, text_color clr)
{
    ::SetConsoleTextAttribute( ::GetStdHandle(STD_OUTPUT_HANDLE), static_cast<::DWORD>(clr) );
    return to;
}


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
	typedef channel_ostream<wchar_t> cons_wostream;
	typedef channel_istream<wchar_t> cons_wistream;
private:
	console();

	static const console* get();
	static void release_console() noexcept;

	static s_write_channel conv_channel(const s_write_channel& ch);

	/// Releases allocated console, and returns original console codepage
	~console() noexcept;

public:

	/// Reset default console output colors
	/// \param in color for input stream
	/// \param out color for output stream
	/// \param err color for error stream
	static void reset_colors(const text_color in,const text_color out,const text_color err) noexcept;

	/// Reset input stream console output color
	/// \param clr new color
	static void reset_in_color(const text_color clr) noexcept;

	/// Reset stdout stream console output color
	/// \param clr new color
	static void reset_out_color(const text_color clr) noexcept;

	/// Reset stderr stream console output color
	/// \param clr new color
	static void reset_err_color(const text_color crl) noexcept;

	/// Returns console input channel
	/// \return console input channel
	/// \throw can throw std::bad_alloc, when out of memory
	static inline s_read_channel in()
	{
		return s_read_channel( get()->cin_, true );
	}

	/// Returns console output channel
	/// \throw can throw std::bad_alloc, when out of memory
	static inline s_write_channel out()
	{
		return s_write_channel( get()->cout_, true );
	}

	static inline s_write_channel err()
	{
		return s_write_channel( get()->cerr_, true );
	}

	/// Returns std::basic_stream<char> with auto-reconverting
	/// UTF-8 multibyte characters into console default UTF-16LE
	static std::ostream& out_stream();

	/// Returns std::basic_stream<char> with ato-reconverting
	/// UTF-8 multibyte characters into console default UTF-16LE
	static std::ostream& error_stream();

	/// Returns std::basic_stream<wchar_t> stream to constole output stream
	static std::wostream& out_wstream();

	/// Returns std::basic_stream<wchar_t> stream to constole error stream
	static std::wostream& error_wstream();

	static std::wistream& in_wstream();

	/// Returns console character set, always UTF-16LE for Windows
	/// \return current console character set
	/// \throw never throws
	static inline const charset& charset() noexcept
	{
		return code_pages::UTF_16LE;
	}
private:
	static std::atomic<console*> _instance;
	static io::critical_section _cs;
	bool need_release_;
	::UINT prev_charset_;
	win::console_channel *cin_;
	win::console_channel *cout_;
	win::console_channel *cerr_;
};

} // namesapce io

#endif // _IO_WIN_CONSOLE_HPP_INCLUDED__
