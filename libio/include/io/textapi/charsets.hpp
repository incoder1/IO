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
#ifndef __CHARSETS_HPP_INCLUDED__
#define __CHARSETS_HPP_INCLUDED__

#include <io/config/libio_config.hpp>

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <array>

#ifdef __HAS_CPP_17
#	include <optional>
#endif // __HAS_CPP_17

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
private:
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
	DECLARE_CHARSET(CP_1251) // Kyrylic (Belarusian,Bulgarian,Russian,Serbian,Ukrainian etc.)
	DECLARE_CHARSET(CP_1252) // latin1 extended
	DECLARE_CHARSET(CP_1253) // Greek
	DECLARE_CHARSET(CP_1254)
	DECLARE_CHARSET(CP_1255)
	DECLARE_CHARSET(CP_1256)
	DECLARE_CHARSET(CP_1257)
	DECLARE_CHARSET(CP_1258)

	/// System wide character set represented by wchar_t,
	/// i.e. UTF-16 for Windows
	/// and UTF-32LE or UTF-32BE for POSIX systems depending on CPU arch
	DECLARE_CHARSET(SYSTEM_WIDE)

	static std::array<const charset*,35> ALL;

public:
	static const charset* ascii() noexcept;
	static const charset* utf7() noexcept;
	static const charset* utf8() noexcept;
	static const charset* utf16le() noexcept;
	static const charset* utf16be() noexcept;
	static const charset* utf32le() noexcept;
	static const charset* utf32be() noexcept;
	static const charset* iso_8859_1() noexcept;
	static const charset* iso_8859_2() noexcept;
	static const charset* iso_8859_3() noexcept;
	static const charset* iso_8859_4() noexcept;
	static const charset* iso_8859_5() noexcept;
	static const charset* iso_8859_6() noexcept;
	static const charset* iso_8859_7() noexcept;
	static const charset* iso_8859_8() noexcept;
	static const charset* iso_8859_9() noexcept;
	static const charset* iso_8859_10() noexcept;
	static const charset* iso_8859_11() noexcept;
	static const charset* iso_8859_12() noexcept;
	static const charset* iso_8859_13() noexcept;
	static const charset* iso_8859_14() noexcept;
	static const charset* iso_8859_15() noexcept;
	static const charset* iso_8859_16() noexcept;
	static const charset* koi8r() noexcept;
	static const charset* koi8u() noexcept;
	static const charset* koi8ru() noexcept;
	static const charset* cp1250() noexcept;
	static const charset* cp1251() noexcept;
	static const charset* cp1252() noexcept;
	static const charset* cp1253() noexcept;
	static const charset* cp1254() noexcept;
	static const charset* cp1255() noexcept;
	static const charset* cp1256() noexcept;
	static const charset* cp1257() noexcept;
	static const charset* cp1258() noexcept;
	static const charset* system_wide() noexcept;

	/// Returns a character set for a iconv name
	/// \param name character set code page code
	/// \return a search result pair where first identifies whether character found and second contains search result
	static std::pair<bool, const charset*> for_name(const char* name) noexcept;

	/// Returns a character set for a code page identifier
	/// \param code character set code page code
	/// \return a search result pair where first identifies whether character found and second contains search result
	static std::pair<bool, const charset*> for_code(uint16_t code) noexcept;

#ifdef __HAS_CPP_17 // __HAS_CPP_17

	/// Returns a character set for a iconv name
	/// \param name character set code page code
	/// \return search result optional
	static std::optional<const charset*> find_by_name(const char* name) noexcept
	{
		auto ret = for_name(name);
		return ret.first ? std::make_optional( std::move(ret.second) ) :  std::nullopt;
	}

	/// Returns a character set for a code page identifier
	/// \param code character set code page code
	static inline std::optional<const charset*> find_by_code(uint16_t code) noexcept
	{
		auto ret = for_code(code);
		return ret.first ? std::make_optional( std::move(ret.second) ) :  std::nullopt;
	}

#endif // __HAS_CPP_17

	/// Returns character set which is default for current operating system API
	/// I.e. UTF-16LE for Winfows or UTF-8 for Linux
	/// \return operating system API default character set
	static const charset* platform_default() noexcept;

	/// Returns character set assigned for this process/application,
	/// i.e. current locale character set
	/// \return current locale charter set
	static const charset* platform_current() noexcept;

private:
	constexpr code_pages() noexcept
	{}
};

#undef DECLARE_CHARSET

} // namespace io


#endif // __CHARSETS_HPP_INCLUDED__
