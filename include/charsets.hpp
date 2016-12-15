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

	constexpr charset(const charset& rhs) noexcept:
		code_(rhs.code_),
		name_(rhs.name_),
		char_max_(rhs.char_max_),
		unicode_(rhs.unicode_)
	{}

	charset& operator=(const charset& rhs) noexcept
	{
		charset(rhs).swap(*this);
		return *this;
	}

	constexpr charset(charset&& rhs) noexcept:
		code_(rhs.code_),
		name_(rhs.name_),
		char_max_(rhs.char_max_),
		unicode_(rhs.unicode_)
	{}

	charset& operator=(charset&& rhs) noexcept
	{
		code_ = std::move(rhs.code_);
		name_ = std::move(rhs.name_);
		char_max_ = std::move(rhs.char_max_);
		unicode_ = std::move(rhs.unicode_);
		return *this;
	}

	constexpr charset(uint16_t code, const char* name, uint8_t char_max, bool unicode) noexcept:
		code_(code),
		name_(name),
		char_max_(char_max),
		unicode_(unicode)
	{}

	inline void swap(charset& rhs) noexcept {
		std::swap(code_,rhs.code_);
		std::swap(name_,rhs.name_);
		std::swap(char_max_,rhs.char_max_);
		std::swap(unicode_,rhs.unicode_);
	}

	/// Returns integer identifier of this character set, number is equal to Win32 id-s
	/// \return charset integer identifier
	constexpr inline uint32_t code() const
	{
		return code_;
	}

	/// Returns string identifier of this character set, names are the same as used by
	/// POSIX libiconv
	/// \return string identifier of this character set
	constexpr inline const char* name() const
	{
		return name_;
	}

	/// Returns maximal width of a single character in bytes
	/// \return maximal width of a single character
	constexpr inline uint8_t char_max_size() const
	{
		return char_max_;
	}

	/// Returns wether this carset is point on an UNICODE representation
	/// \return wether this carset is UNICODE representation
	constexpr inline bool unicode() const
	{
		return unicode_;
	}

	bool operator==(const charset& rhs) const noexcept;

	bool operator!=(const charset& rhs) const noexcept;

private:
	uint16_t code_;
	const char* name_;
	uint8_t char_max_;
	bool unicode_;
};

#define DECLARE_CHARSET(ID) static const charset ID;

/// Holds constants on supported character sets
/// And provides an OS/Locale retriving charsets functions
class IO_PUBLIC_SYMBOL code_pages {
// to avoid externs, enums etc
public:
	/** unicode representations **/
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
	/// Returns character set wich is default for current operating system API
	/// I.e. UTF-16LE for Winfows or UTF-8 for Linux
	/// \return operating system API default charset
	static const charset& platform_default() noexcept;
	/// Returns character set assigned for this process/aplication,
	/// i.e. current user locale charset
	/// \return current locale charset
	static const charset& platform_current() noexcept;
};

#undef DECLARE_CHARSET

} // namespace io

#endif // __CHARSETS_HPP_INCLUDED__
