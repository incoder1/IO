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
#include "stdafx.hpp"
#include "console.hpp"

namespace io {

namespace win {

static inline ::WORD prev_attr(::HANDLE hcons) noexcept {
	::CONSOLE_SCREEN_BUFFER_INFO info;
	::GetConsoleScreenBufferInfo(hcons,&info);
	return info.wAttributes;
}

class attr_swap {
	attr_swap(const attr_swap&) = delete;
	attr_swap& operator=(const attr_swap&) = delete;
public:
	attr_swap(::HANDLE hcons, ::DWORD swattr) noexcept:
		hcons_(hcons),
		prev_( prev_attr(hcons_) )
	{
		::SetConsoleTextAttribute(hcons_, swattr );
	}
	inline ~attr_swap() noexcept
	{
		::SetConsoleTextAttribute(hcons_, prev_ );
	}
private:
	::HANDLE hcons_;
	::DWORD prev_;
};

// console_channel
console_channel::console_channel(::HANDLE hcons, ::WORD attr) noexcept:
	read_write_channel(),
	hcons_(hcons),
	attr_( attr )
{
}

console_channel::~console_channel() noexcept
{
}

std::size_t console_channel::read(std::error_code& err,uint8_t* const buff, std::size_t bytes) const noexcept
{
	attr_swap color_swap(hcons_, attr_);
	assert(bytes % sizeof(::WCHAR) == 0);
	::DWORD result;
	if( ! ::ReadConsoleW(hcons_, (::LPVOID)buff, bytes / sizeof(::WCHAR), &result, nullptr) ) {
		err.assign( ::GetLastError(), std::system_category() );
	}
	return result * sizeof(::WCHAR);
}

std::size_t console_channel::write(std::error_code& err, const uint8_t* buff,std::size_t size) const noexcept
{
	attr_swap color_swap(hcons_, attr_);
	::DWORD result;
	if(! ::WriteConsoleW(hcons_, static_cast<const void*>(buff), size / sizeof(::WCHAR), &result, nullptr ) ) {
		err.assign( ::GetLastError(), std::system_category() );
	}
	return result;
}

} // namespace win

// Console

const console* console::get() {
	static console _instance;
	return &_instance;
}

#ifdef IO_NO_EXCEPTIONS
static void panic(const std::error_code& ec) {
	::HANDLE ferr = ::GetStdHandle(STD_ERROR_HANDLE);
	::DWORD preva = win::prev_attr(ferr);
	::SetConsoleTextAttribute(ferr, 0x0C);
	std::string msg( std::move( ec.message() ) );
	::DWORD written;
	::WriteConsoleA( ferr, msg.data(), msg.length(), &written, nullptr );
	::SetConsoleTextAttribute(ferr, preva);
	std::unexpected();
}
#endif // IO_NO_EXCEPTIONS

static void check_error(const std::error_code& ec)
{
	if(ec)
#ifndef IO_NO_EXCEPTIONS
		throw std::system_error( ec );
#else
		panic( ec );
#endif // IO_NO_EXCEPTIONS
}

console::console():
	need_release_( ::AllocConsole() ),
	prev_charset_( ::GetConsoleCP() ),
	cin_(),
	cout_(),
	cerr_()
{
	std::error_code ec;
	::HANDLE hstdin = ::GetStdHandle(STD_INPUT_HANDLE);
	::HANDLE hstdout = ::GetStdHandle(STD_OUTPUT_HANDLE);
	::HANDLE hstderr =  ::GetStdHandle(STD_ERROR_HANDLE);
	win::console_channel *in = nobadalloc<win::console_channel>::construct(ec, hstdin, win::prev_attr(hstdin) );
	check_error(ec);
	cin_.reset( in , true);
	win::console_channel *out =  nobadalloc<win::console_channel>::construct(ec, hstdout, win::prev_attr(hstdout) );
	check_error(ec);
	cout_.reset(out, true);
	win::console_channel *err =  nobadalloc<win::console_channel>::construct(ec, hstderr,  0x0C );
	check_error(ec);
	cerr_.reset( err, true );
	// both of them returns false, but working as expected
	::SetConsoleCP(CP_WINUNICODE);
	::SetConsoleOutputCP(CP_WINUNICODE);
}

console::~console() noexcept {
	::SetConsoleCP(prev_charset_);
	::SetConsoleOutputCP(prev_charset_);
	// Release console in case of GUI app
	if(need_release_) {
		::FreeConsole();
	}
}

void console::reset_colors(text_color in, text_color out, text_color err) noexcept
{
	console* cons = const_cast<console*>( get() );
	win::console_channel *p = static_cast<win::console_channel*>( cons->cin_.get() );
	p->change_color( static_cast<::DWORD>(in) );
	p = static_cast<win::console_channel*>( cons->cout_.get() );
	p->change_color( static_cast<::DWORD>(out) );
	p = static_cast<win::console_channel*>( cons->cerr_.get() );
	p->change_color( static_cast<::DWORD>(err) );
}

} // namespace io
