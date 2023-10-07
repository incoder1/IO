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
#include "io/core/posix/console.hpp"

namespace io {

namespace posix {

//text color

static const char* NAVY_BLUE = "\033[22;34m";
static const char* NAVY_GREEN = "\033[22;32m";
static const char* NAVY_AQUA = "\033[22;36m";
static const char* NAVY_RED = "\033[22;31m";
static const char* MAGENTA = "\033[22;35m";
static const char* BROWN = "\033[22;33m";
static const char* WHITE = "\033[22;37m";
static const char* GRAY = "\033[01;30m";
static const char* LIGHT_BLUE = "\033[01;34m";
static const char* LIGHT_GREEN = "\033[01;32m";
static const char* LIGHT_AQUA = "\033[01;36m";
static const char* LIGHT_RED = "\033[01;31m";
static const char* PURPLE = "\033[01;35m";
static const char* YELLOW = "\033[01;33m";
static const char* BRIGHT_WHITE = "\033[01;37m";
static const char* RESET = "\033[0m";

static const std::size_t RESET_LEN = io_strlen( RESET );

static const  std::pair<const char*, std::size_t> COLOR_TABLE[15] =
{
	std::make_pair(NAVY_BLUE,io_strlen(NAVY_BLUE)),
	std::make_pair(NAVY_GREEN,io_strlen(NAVY_GREEN)),
	std::make_pair(NAVY_AQUA,io_strlen(NAVY_AQUA)),
	std::make_pair(NAVY_RED,io_strlen(NAVY_RED)),
	std::make_pair(MAGENTA,io_strlen(MAGENTA)),
	std::make_pair(BROWN,io_strlen(BROWN)),
	std::make_pair(WHITE,io_strlen(WHITE)),
	std::make_pair(GRAY,io_strlen(GRAY)),
	std::make_pair(LIGHT_BLUE,io_strlen(LIGHT_BLUE)),
	std::make_pair(LIGHT_GREEN,io_strlen(LIGHT_GREEN)),
	std::make_pair(LIGHT_AQUA,io_strlen(LIGHT_AQUA)),
	std::make_pair(LIGHT_RED,io_strlen(LIGHT_RED)),
	std::make_pair(PURPLE,io_strlen(PURPLE)),
	std::make_pair(YELLOW,io_strlen(YELLOW)),
	std::make_pair(BRIGHT_WHITE,io_strlen(BRIGHT_WHITE))
};

void change_color(int fileno, text_color cl) noexcept
{
	auto clr = COLOR_TABLE[ static_cast<std::size_t>(cl) ];
	::write(fileno, clr.first, clr.second );
}

//  console_channel
console_channel::console_channel(const fd_t stream) noexcept:
	read_write_channel(),
	stream_(stream)
{}

std::size_t console_channel::read(std::error_code& ec,uint8_t* const buff, std::size_t bytes) const noexcept
{
	::ssize_t ret = ::read(stream_, static_cast<void*>(buff), bytes);
	if(ret < 0) {
		ret = 0;
		ec.assign(errno, std::system_category() );
	}
	return static_cast<std::size_t>(ret);
}

std::size_t console_channel::write(std::error_code& ec, const uint8_t* buff,std::size_t size) const noexcept
{
	::ssize_t ret = ::write(stream_, buff, size);
	if(ret < 0) {
		ret = 0;
		ec.assign(errno, std::system_category() );
	}
	return static_cast<std::size_t>(ret);
}

} // namespace posix

// console
console::console():
	in_( new posix::console_channel(STDIN_FILENO) ),
	out_( new posix::console_channel(STDOUT_FILENO) ),
	err_( new posix::console_channel(STDERR_FILENO) )
{}

void console::change_colors(const text_color in,const text_color out,const text_color err) noexcept
{
	posix::change_color(STDIN_FILENO,in);
	posix::change_color(STDOUT_FILENO,out);
	posix::change_color(STDERR_FILENO,err);
}

void console::change_in_color(const text_color clr) noexcept
{
	posix::change_color(STDIN_FILENO,clr);
}

void console::change_out_color(const text_color clr) noexcept
{
	posix::change_color(STDOUT_FILENO,clr);
}

void console::change_err_color(const text_color clr) noexcept
{
	posix::change_color(STDERR_FILENO,clr);
}

void console::reset_in_color() noexcept
{
	::write(STDIN_FILENO, posix::RESET, posix::RESET_LEN);
}

void console::reset_out_color() noexcept
{
	::write(STDOUT_FILENO, posix::RESET, posix::RESET_LEN);
}

void console::reset_err_color() noexcept
{
	::write(STDERR_FILENO, posix::RESET, posix::RESET_LEN);
}

} // namespace io
