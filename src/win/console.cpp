/*
 *
 * Copyright (c) 2016-2017
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#include "stdafx.hpp"
#include "console.hpp"

#include <stdlib.h>

#ifndef CP_WINUNICODE
#define CP_WINUNICODE 1200
#endif // CP_WINUNICODE 1200

namespace io {

namespace win {

static inline ::WORD prev_attr(::HANDLE hcons) noexcept
{
	::CONSOLE_SCREEN_BUFFER_INFO info;
	::GetConsoleScreenBufferInfo(hcons,&info);
	return info.wAttributes;
}

// console_channel
console_channel::console_channel(::HANDLE hcons, ::WORD orig, ::WORD attr) noexcept:
	read_write_channel(),
	hcons_(hcons),
	orig_attr_( orig ),
	attr_( (orig_attr_ & 0xF0) | attr )
{
}

void console_channel::change_color(::DWORD attr) noexcept
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
	if( ! ::ReadConsoleW(hcons_, static_cast<::LPVOID>(buff), bytes / sizeof(::WCHAR), &result, nullptr) )
		err.assign( ::GetLastError(), std::system_category() );
	::SetConsoleTextAttribute(hcons_, orig_attr_ );
	return result * sizeof(::WCHAR);
}

std::size_t console_channel::write(std::error_code& err, const uint8_t* buff,std::size_t size) const noexcept
{
	::DWORD result;
	::SetConsoleTextAttribute(hcons_, attr_ );
	if(! ::WriteConsoleW(hcons_, static_cast<const void*>(buff), size / sizeof(::WCHAR), &result, nullptr ) )
		err.assign( ::GetLastError(), std::system_category() );
	::SetConsoleTextAttribute(hcons_, orig_attr_ );
	return result * sizeof(::WCHAR);
}

} // namespace win

// Console

std::ostream& console::out_stream()
{
	static cons_ostream _outs( console::conv_channel( console::out() ) );
	return _outs;
}

std::ostream& console::error_stream()
{
	static cons_ostream _errs( console::conv_channel( console::err() ) );
	return _errs;
}

std::wostream& console::out_wstream()
{
	static cons_wostream _wcouts( console::out() );
	return _wcouts;
}

std::wostream& console::error_wstream()
{
	static cons_wostream _wcerrs( console::err()  );
	return _wcerrs;
}

std::wistream& console::in_wstream()
{
	static cons_wistream _wcins( console::in(), 256 );
	return _wcins;
}

console::console():
	need_release_( ::AllocConsole() ),
	prev_charset_( ::GetConsoleCP() ),
	cin_(nullptr),
	cout_(nullptr),
	cerr_(nullptr)
{
	::HANDLE hstdin = ::GetStdHandle(STD_INPUT_HANDLE);
	::HANDLE hstdout = ::GetStdHandle(STD_OUTPUT_HANDLE);
	::HANDLE hstderr =  ::GetStdHandle(STD_ERROR_HANDLE);
	::WORD origing_attr = win::prev_attr(hstdout);
	cin_ = new win::console_channel( hstdin, origing_attr, origing_attr );
	intrusive_ptr_add_ref( cin_ );
	cout_ = new win::console_channel( hstdout, origing_attr, origing_attr);
	intrusive_ptr_add_ref( cout_ );
	cerr_ =  new win::console_channel( hstderr, origing_attr, 0x0C);
	intrusive_ptr_add_ref( cerr_ );
	// both of them returns false, but working as expected
	::SetConsoleCP(CP_WINUNICODE);
	::SetConsoleOutputCP(CP_WINUNICODE);
}

console::~console() noexcept
{
	delete cin_;
	delete cout_;
	delete cerr_;

	::SetConsoleCP(prev_charset_);
	::SetConsoleOutputCP(prev_charset_);
	// Release console in case of GUI app
	if(need_release_)
		::FreeConsole();
}

void console::reset_colors(const text_color in,const text_color out,const text_color err) noexcept
{
	console* cons = const_cast<console*>( get() );
	lock_guard lock(_cs);
	cons->cin_->change_color( static_cast<::DWORD>(in) );
	cons->cout_->change_color( static_cast<::DWORD>(out) );
	cons->cerr_->change_color( static_cast<::DWORD>(err) );
}

void console::reset_in_color(const text_color clr) noexcept
{
	console* cons = const_cast<console*>( get() );
	lock_guard lock(_cs);
	cons->cin_->change_color( static_cast<::DWORD>(clr) );
}

void console::reset_out_color(const text_color clr) noexcept
{
	console* cons = const_cast<console*>( get() );
	lock_guard lock(_cs);
	cons->cout_->change_color( static_cast<::DWORD>(clr) );
}

void console::reset_err_color(const text_color clr) noexcept
{
	console* cons = const_cast<console*>( get() );
	lock_guard lock(_cs);
	cons->cerr_->change_color( static_cast<::DWORD>(clr) );
}

s_write_channel console::conv_channel(const s_write_channel& ch)
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


std::atomic<console*> console::_instance( nullptr );
io::critical_section  console::_cs;

void console::release_console() noexcept
{
	console* inst = _instance.load( std::memory_order_acquire );
	inst->~console();
	memory_traits::free( inst );
	_instance.store(nullptr, std::memory_order_release );
}

const console* console::get()
{
	console *tmp = _instance.load( std::memory_order_relaxed );
	if( nullptr == tmp ) {
		tmp = _instance.load( std::memory_order_acquire );
		lock_guard lock(_cs);
		if( nullptr == tmp ) {
			void *raw = memory_traits::malloc( sizeof(console) );
			if(nullptr != raw) {
				std::atexit( &console::release_console );
				tmp = new (raw) console();
				_instance.store( tmp, std::memory_order_release );
			}
			std::atomic_thread_fence( std::memory_order_release );
		}
	}
	return tmp;
}

} // namespace io
