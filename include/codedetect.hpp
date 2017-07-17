/*
 * Copyright (c) 2016
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#ifndef __IO_CODEDETECT_HPP_INCLUDED__
#define __IO_CODEDETECT_HPP_INCLUDED__

#include "config.hpp"
#include "charsets.hpp"
#include <ostream>

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

namespace io {

class IO_PUBLIC_SYMBOL utf8_bom  {
private:
	static const uint8_t* DATA;
public:
	static inline const uint8_t* data() noexcept {
		return DATA;
	}
	static constexpr inline std::size_t len() noexcept {
		return 3;
	}
	static inline bool is(const uint8_t* str) noexcept {
		return 0 == io_memcmp(DATA, str, len() );
	}
	static inline bool is(const char* str) {
		return is( reinterpret_cast<const uint8_t*>(str) );
	}
};

class IO_PUBLIC_SYMBOL utf_16le_bom {
private:
	static const uint8_t *DATA;
public:
	static inline const uint8_t* data() noexcept {
		return DATA;
	}
	static constexpr inline std::size_t len() noexcept {
		return 2;
	}
	static inline bool is(const uint8_t* str) noexcept {
		return 0 == io_memcmp( DATA, str, len() );
	}
	static inline bool is(const char* str) {
		return is( reinterpret_cast<const uint8_t*>(str) );
	}
};

class IO_PUBLIC_SYMBOL utf_32le_bom {
private:
	static const uint8_t *DATA;
public:
	static inline const uint8_t* data() noexcept {
		return DATA;
	}
	static constexpr inline std::size_t len() noexcept {
		return 4;
	}
	static inline bool is(const uint8_t* str) noexcept {
		return 0 == io_memcmp(DATA,str,len());
	}
	static inline bool is(const char* str) {
		return is( reinterpret_cast<const uint8_t*>(str) );
	}
};

class IO_PUBLIC_SYMBOL utf_16be_bom {
private:
	static const uint8_t *DATA;
public:
	static inline const uint8_t* data() noexcept {
		return DATA;
	}
	static constexpr inline std::size_t len() noexcept {
		return 2;
	}
	static inline bool is(const uint8_t* str) noexcept {
		return 0 == io_memcmp(DATA,str,len());
	}
	static inline bool is(const char* str) {
		return is( reinterpret_cast<const uint8_t*>(str) );
	}
};

class IO_PUBLIC_SYMBOL utf_32be_bom {
private:
	static const uint8_t *DATA;
public:
	static inline const uint8_t* data() noexcept {
		return DATA;
	}
	static constexpr inline std::size_t len() noexcept {
		return 4;
	}
	static inline bool is(const uint8_t* str) noexcept {
		return 0 == io_memcmp(DATA,str,len());
	}
	static inline bool is(const char* str) {
		return is( reinterpret_cast<const uint8_t*>(str) );
	}
};

enum class unicode_cp {
	utf8,
	utf_16le,
	utf_16be,
	utf_32le,
	utf_32be,
	not_detected
};

template<typename _char_type,class _trailts_type = std::char_traits<_char_type> >
inline std::basic_ostream<_char_type,_trailts_type>& operator<<(std::basic_ostream<_char_type, _trailts_type>& os, unicode_cp bom) noexcept {
	const uint8_t *bomd;
	std::size_t boml;
	switch(bom) {
	case unicode_cp::not_detected:
		return os;
	case unicode_cp::utf8:
		bomd = utf8_bom::data();
		boml = utf8_bom::len();
		break;
	case unicode_cp::utf_16le:
		bomd = utf_16le_bom::data();
		boml = utf_16le_bom::len();
		break;
	case unicode_cp::utf_32le:
		bomd = utf_32le_bom::data();
		boml = utf_32le_bom::len();
		break;
	case unicode_cp::utf_16be:
		bomd = utf_16be_bom::data();
		boml = utf_16be_bom::len();
		break;
	case unicode_cp::utf_32be:
		bomd = utf_32be_bom::data();
		boml = utf_32be_bom::len();
		break;
	}
	os.write( reinterpret_cast<const _char_type*>(bomd), boml);
	return os;
}

class unicode_charset_detector {
	unicode_charset_detector(const unicode_charset_detector&) = delete;
	unicode_charset_detector& operator=(const unicode_charset_detector&) = delete;
public:
	static unicode_cp detect_or_guess_charset(const uint8_t* where) {
		// try to detect first
		unicode_cp result = detect_by_bom(where);
		if(unicode_cp::not_detected == result) {
			result = guess_charset(where);
		}
		return result;
	}
	~unicode_charset_detector() noexcept = default;
private:

constexpr unicode_charset_detector() noexcept = default;

// TODO:: Refactor
static unicode_cp detect_by_bom(const uint8_t * bom)
{
	if( utf8_bom::is(bom) ) {
		return unicode_cp::utf8;
	}
	if( 0xFF == *bom && 0xFE == *(bom+1) ) {
		if( 0x00 == *(bom+2) && 0x00 == *(bom+3) ) {
			return unicode_cp::utf_32le;
		} else {
			return unicode_cp::utf_16le;
		}
	}
	if(0x00 == *(bom) && 0x00 == *(bom+1) && 0xFE == *(bom+2) && 0xFF == *(bom+3) ) {
		return unicode_cp::utf_32be;
	}
	if( 0xFE == *bom && 0xFF == *(bom+1) ) {
		return unicode_cp::utf_16be;
	}
	return unicode_cp::not_detected;
}

template<typename int_type>
static inline bool bmchk(int_type mask, int_type i) {
	return mask == (i & mask);
}

static inline bool is_utf32le(uint32_t dword) {
	return bmchk(static_cast<uint32_t>(0x000000FF),dword) || ( (dword >> 8) > 0 ) || ( (dword >> 16) > 0 );
}

static inline bool is_utf32be(uint32_t dword) {
	return bmchk(static_cast<uint32_t>(0xFF000000),dword) || ( (dword << 8) > 0 ) || ( (dword << 16) > 0 );
}

static inline bool is_utf16le(uint16_t word) {
	return bmchk(static_cast<uint16_t>(0x00FF),word) || ( (word >> 8) > 0 );
}

static inline bool is_utf16be(uint16_t word) {
	return bmchk(static_cast<uint16_t>(0xFF00),word) || ( (word << 8) > 0 ) ;
}

static bool check_utf8_mb(uint8_t b) {
	return 0xC0 == b || 0xE0 == b || 0xF0 == b || 0xF8 == b || 0xFC == b;
}

static unicode_cp guess_charset(const uint8_t* bytes) {
	if( check_utf8_mb( *bytes) ) {
		// UTF8 multibyte detected
		return unicode_cp::utf8;
	}
	uint32_t i = *reinterpret_cast<const uint8_t*>(bytes);
	if( is_utf32le(i) ) {
		// it is looks like an UTF-32LE
		return unicode_cp::utf_32le;
	}

	if( is_utf32be(i) ) {
		// it is looks like an UTF-32BE
        return unicode_cp::utf_32be;
	}

	uint16_t s = *reinterpret_cast<const uint16_t*>(bytes);

	if( is_utf16le(s) ) {
		// it is looks like UTF-16LE
		return unicode_cp::utf_16le;
	}

	if( is_utf16be(s) ) {
		// it is looks like UTF-16be
		return unicode_cp::utf_16be;
	}
	// mind be a UTF-8 or an ASSCII/ ISO latin1 which is ok for 'char' type
	// as well as strings interpretted as UTF-8
	if( *bytes <= 127 ) {
		return unicode_cp::utf8;
	}
	// this library no longer have any ideas what the charset is
	// you may try an external detector as:
	//   * IBM ICU has an charset detector
	//   * Mozilla universal charset detector, part of the Mozilla Firefox
	//   * Microsoft Mlang has an charset detector - only for windows with
	//      internet explorer installed, you need to manually
	//      generate header's for COM components
	return unicode_cp::not_detected;
}

};

class charset_detector:public object
{
public:
	charset_detector(const charset_detector&) = delete;
	charset_detector& operator=(const charset_detector&) = delete;
private:
	const charset* detect(std::error_code ec&, const uint8_t* sequence) noexcept;
};

} // namespace io


#endif // __IO_CODEDETECT_HPP_INCLUDED__
