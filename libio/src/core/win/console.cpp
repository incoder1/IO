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
#include "stdafx.hpp"
#include "io/core/win/console.hpp"

#ifndef CP_WINUNICODE
#define CP_WINUNICODE 1200
#endif // CP_WINUNICODE 1200

namespace io {

namespace win {

// console_channel
console_channel::console_channel(::HANDLE hcons) noexcept:
	read_write_channel(),
	hcons_(hcons)
{
}

console_channel::~console_channel() noexcept
{
}

std::size_t console_channel::read(std::error_code& err,uint8_t* const buff, std::size_t bytes) const noexcept
{
	assert(bytes % sizeof(::WCHAR) == 0);
	::DWORD result;
	if( ! ::ReadConsoleW(hcons_, static_cast<::LPVOID>(buff), ::DWORD(bytes / sizeof(::WCHAR)), &result, nullptr) )
		err.assign( ::GetLastError(), std::system_category() );
	return result * sizeof(::WCHAR);
}

std::size_t console_channel::write(std::error_code& err, const uint8_t* buff,std::size_t size) const noexcept
{
	::DWORD result;
	if(! ::WriteConsoleW(hcons_, static_cast<const void*>(buff), ::DWORD(size / sizeof(::WCHAR)), &result, nullptr ) )
		err.assign( ::GetLastError(), std::system_category() );
	return result * sizeof(::WCHAR);
}



s_read_channel IO_PUBLIC_SYMBOL conv_read_channel(const s_read_channel& ch)
{
	std::error_code ec;
	s_charset_converter conv = charset_converter::open(ec,code_pages::utf8(),code_pages::utf16le());
	io::check_error_code( ec );
	s_read_channel result =  ch ; // conv_read_channel::open(ec, ch, conv);
	io::check_error_code( ec );
	return result;
}


} // namespace win

// Console

::HANDLE console::in_handle() noexcept
{
	return ::GetStdHandle(STD_INPUT_HANDLE);
}

::HANDLE console::out_handle() noexcept
{
	return ::GetStdHandle(STD_OUTPUT_HANDLE);
}

::HANDLE console::err_handle() noexcept
{
	return ::GetStdHandle(STD_ERROR_HANDLE);
}

::WORD console::current_stream_attributes(::HANDLE hcons) noexcept
{
	::CONSOLE_SCREEN_BUFFER_INFO info;
	::GetConsoleScreenBufferInfo(hcons,&info);
	return info.wAttributes;
}

console::console():
	in_(nullptr),
	out_(nullptr),
	err_(nullptr),
	prev_charset_( ::GetConsoleCP() ),
	default_in_attr_( current_stream_attributes(in_handle()) ),
	default_out_attr_( current_stream_attributes(out_handle()) ),
	default_err_attr_( current_stream_attributes(err_handle()) ),
	need_release_( ::AllocConsole() )
{
	in_.reset( new win::console_channel(in_handle()) );
	out_.reset( new win::console_channel(out_handle()) );
	err_.reset( new win::console_channel(err_handle()) );
	// both of them returns false, but working as expected
	::SetConsoleCP(CP_WINUNICODE);
	::SetConsoleOutputCP(CP_WINUNICODE);
}

console::~console() noexcept
{
	::SetConsoleCP(prev_charset_);
	::SetConsoleOutputCP(prev_charset_);
	// Release console in case of GUI app
	if(need_release_)
		::FreeConsole();
}

void console::change_colors(const text_color in,const text_color out,const text_color err) noexcept
{
	change_in_color(in);
	change_out_color(out);
	change_err_color(err);
}

void console::change_in_color(const text_color clr) noexcept
{
	::WORD attr = (default_in_attr_ & 0xF0) |  static_cast<::DWORD>(clr);
	::SetConsoleTextAttribute(in_handle(), attr);
}

void console::change_out_color(const text_color clr) noexcept
{
	::WORD attr = (default_out_attr_ & 0xF0) |  static_cast<::DWORD>(clr);
	::SetConsoleTextAttribute(out_handle(), attr);
}

void console::change_err_color(const text_color clr) noexcept
{
	::WORD attr = (default_err_attr_ & 0xF0) |  static_cast<::DWORD>(clr);
	::SetConsoleTextAttribute(err_handle(), attr);
}

void console::reset_in_color() noexcept
{
	::SetConsoleTextAttribute(in_handle(), default_in_attr_);
}

void console::reset_out_color() noexcept
{
	::SetConsoleTextAttribute(out_handle(), default_out_attr_);
}

void console::reset_err_color() noexcept
{
	::SetConsoleTextAttribute(err_handle(), default_err_attr_);
}

} // namespace io
