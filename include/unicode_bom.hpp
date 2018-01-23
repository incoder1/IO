/*
 * Copyright (c) 2016-2017
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#ifndef __IO_UNICODE_BOM_HPP_INCLUDED__
#define __IO_UNICODE_BOM_HPP_INCLUDED__

#include "config.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include "charsets.hpp"
#include <ostream>

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

namespace io {

/// UNICODE UTF-8 byte order mark primitive
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

/// UNICODE UTF-16LE byte order mark primitive
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

/// UNICODE UTF-32LE byte order mark primitive
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

/// UNICODE UTF-16BE byte order mark primitive
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

/// UNICODE UTF-32BE byte order mark primitive
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

/// UNICODE representation types enumeration
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

inline unicode_cp detect_by_bom(const uint8_t * bom)
{
	if( utf8_bom::is(bom) )
		return unicode_cp::utf8;
	if( utf_16le_bom::is(bom) ) {
		if( utf_32le_bom::is(bom) )
			return unicode_cp::utf_32le;
		return unicode_cp::utf_16le;
	}
	if( utf_16be_bom::is(bom) )
		return unicode_cp::utf_16be;
	if( utf_32be_bom::is(bom) )
		return unicode_cp::utf_32be;
	return unicode_cp::not_detected;
}

} // namespace io


#endif // __IO_UNICODE_BOM_HPP_INCLUDED__
