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
#include "stdafx.hpp"
#include "console.hpp"


#ifndef CP_WINUNICODE
#define CP_WINUNICODE 1200
#endif // CP_WINUNICODE 1200

namespace io {

namespace win {

// console_channel
console_channel::console_channel(::HANDLE hcons, ::WORD orig, ::WORD attr) noexcept:
	read_write_channel(),
	hcons_(hcons),
	orig_attr_( orig ),
	attr_( (orig_attr_ & 0xF0) | attr )
{
}

void console_channel::change_color(::WORD attr) noexcept
{
	attr_ = (orig_attr_ & 0xF0) | attr;
}

console_channel::~console_channel() noexcept
{
}

std::size_t console_channel::read(std::error_code& err,uint8_t* const buff, std::size_t bytes) const noexcept
{
	assert(bytes % sizeof(::WCHAR) == 0);
	::DWORD result;
	::SetConsoleTextAttribute(hcons_, attr_ );
	if( ! ::ReadConsoleW(hcons_, static_cast<::LPVOID>(buff), ::DWORD(bytes / sizeof(::WCHAR)), &result, nullptr) )
		err.assign( ::GetLastError(), std::system_category() );
	::SetConsoleTextAttribute(hcons_, orig_attr_ );
	return result * sizeof(::WCHAR);
}

std::size_t console_channel::write(std::error_code& err, const uint8_t* buff,std::size_t size) const noexcept
{
	::DWORD result;
	::SetConsoleTextAttribute(hcons_, attr_ );
	if(! ::WriteConsoleW(hcons_, static_cast<const void*>(buff), ::DWORD(size / sizeof(::WCHAR)), &result, nullptr ) )
		err.assign( ::GetLastError(), std::system_category() );
	::SetConsoleTextAttribute(hcons_, orig_attr_ );
	return result * sizeof(::WCHAR);
}

} // namespace win

// Console
console::console():
	in_(nullptr),
	out_(nullptr),
	err_(nullptr),
	prev_charset_( ::GetConsoleCP() ),
	need_release_( ::AllocConsole() )
{
	::HANDLE hstdin = ::GetStdHandle(STD_INPUT_HANDLE);
	::HANDLE hstdout = ::GetStdHandle(STD_OUTPUT_HANDLE);
	::HANDLE hstderr =  ::GetStdHandle(STD_ERROR_HANDLE);
	::WORD origing_attr = win::prev_attr(hstdout);
	in_.reset( new win::console_channel( hstdin, origing_attr, origing_attr )  );
	out_.reset( new win::console_channel( hstdout, origing_attr, origing_attr) );
	err_.reset( new win::console_channel( hstderr, origing_attr, 0x0C) );
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

void console::reset_colors(const text_color in,const text_color out,const text_color err) noexcept
{
	/*
	in_->change_color( static_cast<::DWORD>(in) );
	out_->change_color( static_cast<::DWORD>(out) );
	err_->change_color( static_cast<::DWORD>(err) );
	*/
}

void console::reset_in_color(const text_color clr) noexcept
{
    /*
	console* cons = const_cast<console*>( get() );
	lock_guard lock(_cs);
	cin_->change_color( static_cast<::DWORD>(clr) );
	*/
}

void console::reset_out_color(const text_color clr) noexcept
{
	// cout_->change_color( static_cast<::DWORD>(clr) );
}

void console::reset_err_color(const text_color clr) noexcept
{
	// console* cons = const_cast<console*>( get() );
	// lock_guard lock(_cs);
	// cerr_->change_color( static_cast<::DWORD>(clr) );
}

s_write_channel console::conv_w_channel(const s_write_channel& ch)
{
	std::error_code ec;
	s_code_cnvtr conv = code_cnvtr::open(ec,
										code_pages::UTF_8,
										code_pages::UTF_16LE,
										cnvrt_control::discard_on_failing_chars);
	io::check_error_code( ec );
	s_write_channel result = conv_write_channel::open(ec, ch, conv);
	io::check_error_code( ec );
	return result;
}

s_read_channel console::conv_r_channel(const s_read_channel& ch)
{
    std::error_code ec;
    s_code_cnvtr conv = code_cnvtr::open(ec,
										code_pages::UTF_8,
										code_pages::UTF_16LE,
										cnvrt_control::discard_on_failing_chars);
	io::check_error_code( ec );
	s_read_channel result = conv_read_channel::open(ec, ch, conv);
	io::check_error_code( ec );
	return result;
}

} // namespace io
