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
#include "stdafx.hpp"
#include "io/textapi/unicode.hpp"

namespace io {

namespace utf8 {

static void mbtochar32_noshift(char32_t& dst, const char* src,std::size_t& len)
{
	len = 0;
	switch( mblen(src) ) {
	case 1:
		dst = static_cast<char32_t>( *src );
		len = 1;
		break;
	case 2:
		dst = decode2( src );
		len = 2;
		break;
	case 3:
		dst = decode3( src );
		len = 3;
		break;
	case 4:
		dst = decode4( src );
		len = 4;
		break;
	default:
		dst = U'\0';
		break;
	}
}

#ifdef _MSC_VER
IO_PUBLIC_SYMBOL const char* mbtochar32(char32_t& dst, const char* src) noexcept
#else
const char* IO_PUBLIC_SYMBOL mbtochar32(char32_t& dst, const char* src) noexcept
#endif
{
	std::size_t shift;
	mbtochar32_noshift(dst, src, shift);
	return src+shift;
}

#ifdef _MSC_VER
IO_PUBLIC_SYMBOL const u8char_t* mbtochar32(char32_t& dst, const u8char_t* src) noexcept
#else
const u8char_t* IO_PUBLIC_SYMBOL mbtochar32(char32_t& dst, const u8char_t* src) noexcept
#endif
{
	std::size_t shift;
	mbtochar32_noshift(dst, reinterpret_cast<const char*>(src), shift);
	return src+shift;
}

} // namespace utf8

static constexpr uint8_t _u8_bom[3]    = {0xEF,0xBB,0xBF};
static constexpr uint8_t _u16le_bom[2] = {0xFF,0xFE};
static constexpr uint8_t _u32le_bom[4] = {0xFF,0xFE,0x00,0x00};
static constexpr uint8_t _u16be_bom[2] = {0xFE,0xFF};
static constexpr uint8_t _u32be_bom[4] = {0x00,0x00,0xFE,0xFF};

const uint8_t* utf8_bom::DATA     = _u8_bom;
const uint8_t* utf_16le_bom::DATA = _u16le_bom;
const uint8_t* utf_16be_bom::DATA = _u16be_bom;
const uint8_t* utf_32le_bom::DATA = _u32le_bom;
const uint8_t* utf_32be_bom::DATA = _u32be_bom;



} // namespace io
