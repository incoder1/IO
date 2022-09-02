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
#ifndef __IO_CONSOLE_HPP_INCLUDED__
#define __IO_CONSOLE_HPP_INCLUDED__

#include "config.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#ifdef __IO_WINDOWS_BACKEND__
#	include "win/console.hpp"
#elif defined(__IO_POSIX_BACKEND__)
#	include "posix/console.hpp"
#endif // __IO_WINDOWS_BACKEND__

namespace io {

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

} // namespace io

#endif // __IO_CONSOLE_HPP_INCLUDED__

