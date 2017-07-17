#ifndef __IO_POSIX_CONSOLE_HPP_INCLUDED__
#define __IO_POSIX_CONSOLE_HPP_INCLUDED__

#include <config.hpp>

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <atomic>
#include <memory>

#include <channels.hpp>
#include <stream.hpp>

#include "criticalsection.hpp"

namespace io {

// 30 	    31 	    32 	    33 	    34 	    35 	        36 	    37
// Black 	Red 	Green 	Yellow 	Blue 	Magenta 	Cyan 	White
enum class text_color {
	navy_blue,
	navy_green,
	navy_aqua,
	navy_red,
	magenta,
	brown,
	white,
	gray,
	light_blue,
	light_green,
	light_aqu,
	light_red,
	light_purple,
	yellow,
	bright_white
};

class console
{
	private:
		console();
	private:
		static std::atomic<console*> _instance;
		static critical_section _init_cs;
};

} // namespace io

#endif // __IO_POSIX_CONSOLE_HPP_INCLUDED__
