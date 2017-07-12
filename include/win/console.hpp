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
#include <memory>

#include <wincon.h>
#include <tchar.h>

#include <text.hpp>
#include <stream.hpp>

#include "criticalsection.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#ifndef _CU
#	define _CU(quote) L##quote
#endif // __LOCALE_TEXT


namespace io {

class console;

namespace win {

class IO_PUBLIC_SYMBOL console_channel final: public read_write_channel
{
	console_channel(const console_channel&) = delete;
	console_channel& operator=(const console_channel&) = delete;
	console_channel(console_channel&&) = delete;
	console_channel& operator=(console_channel&&) = delete;
public:
	console_channel(::HANDLE hcons, ::WORD attr) noexcept;
	virtual ~console_channel() noexcept override;
	virtual std::size_t read(std::error_code& err,uint8_t* const buff, std::size_t bytes) const noexcept override;
	virtual std::size_t write(std::error_code& err, const uint8_t* buff,std::size_t size) const noexcept override;
private:
	friend class io::console;
	inline void change_color(::DWORD attr)
	{
		attr_ = attr;
	}
	::HANDLE hcons_;
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



/// \brief System console (terminal window) low level access API.
/// Windows based implementation
/// Console is not a replacement for C++ iostreams, it is for compatibility
/// with channels API. Unlike iostreams this is low level access interface to
/// the console OS system calls without locales and formating support.
/// Console is usefull to implement a software like POSIX 'cat','more' etc. when you
/// only need to output some raw character data into console screen, and don't care
/// about content.
/// Console is UNICODE (UTF-16LE) only! I.e. supports multi-languages text at time
class IO_PUBLIC_SYMBOL console {
public:
	typedef ::HANDLE native_id;
	typedef channel_ostream<char> cons_ostream;
private:
	console();

	static const console* get();
	static s_write_channel conv_channel(const s_write_channel& ch);

public:

	static void reset_colors(text_color in, text_color out, text_color err) noexcept;

	/// Releases allocated console, and returns original console codepage
	~console() noexcept;

	/// Returns console input channel
	/// \return console input channel
	/// \throw can throw std::bad_alloc, when out of memory
	static inline s_read_channel in() {
		return s_read_channel( get()->cin_ );
	}

	/// Returns console output channel
	/// \throw can throw std::bad_alloc, when out of memory
	static inline s_write_channel out() {
		return s_write_channel( get()->cout_ );
	}

	static inline s_write_channel err() {
		return s_write_channel( get()->cerr_ );
	}

	static cons_ostream& out_stream()
	{
		return _out;
	}

	static cons_ostream& error_stream()
	{
		return _err;
	}

	/// Returns console character set, always UTF-16LE for Windows
	/// \return current console character set
	/// \throw never throws
	static inline const charset& charset() noexcept {
		return code_pages::UTF_16LE;
	}

  private:
	 bool need_release_;
	 ::UINT prev_charset_;
	 s_read_channel cin_;
	 s_write_channel cout_;
	 s_write_channel cerr_;
	 static cons_ostream _out;
	 static cons_ostream _err;
};

//typedef channel_istream<char> cistream;
typedef channel_ostream<wchar_t> const_wstream;
//typedef channel_istream<wchar_t> wcistream;

/*
wcostream& IO_PUBLIC_SYMBOL wout_stream();
wcostream& IO_PUBLIC_SYMBOL werr_stream();
*/

} // namesapce io

#endif // _IO_WIN_CONSOLE_HPP_INCLUDED__
