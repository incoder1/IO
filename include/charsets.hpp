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
#ifndef __CHARSETS_HPP_INCLUDED__
#define __CHARSETS_HPP_INCLUDED__

#include "config.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

namespace io {

/// \brief A named mapping for the character set code page
class IO_PUBLIC_SYMBOL charset {
public:

	charset(const charset& rhs) = default;
	charset& operator=(const charset& rhs) = default;
	charset(charset&& rhs) noexcept = default;
	charset& operator=(charset&& rhs) noexcept = default;

	explicit constexpr charset(uint16_t code, const char* name, uint8_t char_max, bool unicode) noexcept:
		name_(name),
		code_(code),
		char_max_(char_max),
		unicode_(unicode)
	{}

	constexpr charset() noexcept:
		charset(0,nullptr,0,false)
	{}

	/// Checks this charset points to a known code page
	explicit operator bool() const noexcept {
		return char_max_ != 0 && code_ != 0;
	}

	/// Returns integer identifier of this character set, number is equal to Win32 id-s
	/// \return charset integer identifier
	constexpr inline uint32_t code() const {
		return code_;
	}

	/// Returns string identifier of this character set, names are the same as used by
	/// POSIX iconv
	/// \return string identifier of this character set
	constexpr inline const char* name() const {
		return name_;
	}

	/// Returns maximal width of a single character in bytes
	/// \return maximal width of a single character
	constexpr inline uint8_t char_max_size() const {
		return char_max_;
	}

	/// Returns whether this character set is point on an UNICODE representation code page
	/// \return whether this character set is UNICODE representation
	constexpr inline bool unicode() const {
		return unicode_;
	}

	/// Checks character set equality
	bool operator==(const charset& rhs) const noexcept {
		return code_ == rhs.code_;
	}

	/// Checks character set equality
	bool operator!=(const charset& rhs) const noexcept {
		return code_ != rhs.code_;
	}

private:
	inline void swap(charset& with) noexcept {
		std::swap(unicode_, with.unicode_);
		std::swap(char_max_, with.char_max_);
		std::swap(code_, with.code_);
		std::swap(name_, with.name_);
	}
private:
	const char* name_;
	uint16_t code_;
	uint8_t char_max_;
	bool unicode_;
};

#define DECLARE_CHARSET(ID) static const charset ID;

/// Enumeration of supported character sets constants
class IO_PUBLIC_SYMBOL code_pages {
	code_pages(const code_pages&) = delete;
	code_pages& operator=(code_pages&) = delete;
// to avoid externs, enums etc
public:
	/** UNICODE representations **/
	DECLARE_CHARSET(UTF_8)
	DECLARE_CHARSET(UTF_16LE)
	DECLARE_CHARSET(UTF_16BE)
	DECLARE_CHARSET(UTF_32BE)
	DECLARE_CHARSET(UTF_32LE)
	DECLARE_CHARSET(UTF_7)
	/** one byte code pages **/
	DECLARE_CHARSET(ASCII)
// ISO standards ASCII comp
	DECLARE_CHARSET(ISO_8859_1)
	DECLARE_CHARSET(ISO_8859_2)
	DECLARE_CHARSET(ISO_8859_3)
	DECLARE_CHARSET(ISO_8859_4)
	DECLARE_CHARSET(ISO_8859_5)
	DECLARE_CHARSET(ISO_8859_6)
	DECLARE_CHARSET(ISO_8859_7)
	DECLARE_CHARSET(ISO_8859_8)
	DECLARE_CHARSET(ISO_8859_9)
	DECLARE_CHARSET(ISO_8859_10)
	DECLARE_CHARSET(ISO_8859_11)
	DECLARE_CHARSET(ISO_8859_12)
	DECLARE_CHARSET(ISO_8859_13)
	DECLARE_CHARSET(ISO_8859_14)
	DECLARE_CHARSET(ISO_8859_15)
	DECLARE_CHARSET(ISO_8859_16)
// Cyrillic Unix
	DECLARE_CHARSET(KOI8_R)  // Unix Kyrylic single byte for Belarusian
	DECLARE_CHARSET(KOI8_U)  // Unix Kyrylic soveit DOS single byte for Ukrainian
	DECLARE_CHARSET(KOI8_RU) // Unix Kyrylic soveit DOS single byte for Russian
// Windows national code pages for the an alphabet based languages
	DECLARE_CHARSET(CP_1250) // latin 1
	DECLARE_CHARSET(CP_1251) // latin1 extended
	DECLARE_CHARSET(CP_1252) // Kyrylic (Belarusian,Bulgarian,Russian,Serbian,Ukrainian etc.)
	DECLARE_CHARSET(CP_1253) // Greek
	DECLARE_CHARSET(CP_1254)
	DECLARE_CHARSET(CP_1255)
	DECLARE_CHARSET(CP_1256)
	DECLARE_CHARSET(CP_1257)
	DECLARE_CHARSET(CP_1258)

	/// Returns a character set for a iconv name
	static std::pair<bool, charset> for_name(const char* name) noexcept;

	/// Returns character set which is default for current operating system API
	/// I.e. UTF-16LE for Winfows or UTF-8 for Linux
	/// \return operating system API default character set
	static const charset& platform_default() noexcept;

	/// Returns character set assigned for this process/application,
	/// i.e. current locale character set
	/// \return current locale charter set
	static charset platform_current() noexcept;
private:
	constexpr code_pages() noexcept
	{}
};

#undef DECLARE_CHARSET

} // namespace io


#endif // __CHARSETS_HPP_INCLUDED__
