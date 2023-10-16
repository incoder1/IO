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
#include "io/textapi/detail/french_model.hpp"

namespace io {

namespace detail {

/* Character Mapping Table:
 * ILL: illegal character.
 * CTR: control character specific to the charset.
 * RET: carriage/return.
 * SYM: symbol (punctuation) that does not belong to word.
 * NUM: 0 - 9.
 *
 * Other characters are ordered by probabilities
 * (0 is the most common character in the language).
 *
 * Orders are generic to a language. So the codepoint with order X in
 * CHARSET1 maps to the same character as the codepoint with the same
 * order X in CHARSET2 for the same language.
 * As such, it is possible to get missing order. For instance the
 * ligature of 'o' and 'e' exists in ISO-8859-15 but not in ISO-8859-1
 * even though they are both used for French. Same for the euro sign.
 */
static uint8_t WIN1252_FRENCH_CHAR_TO_ORDER_MAP[] = {
	0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFC,0xFE,0xFE,0xFC,0xFE,0xFE,
	0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,
	0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,
	0xFB,0xFB,0xFB,0xFB,0xFB,0xFB,0xFB,0xFB,0xFB,0xFB,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,
	0xFD,0x01,0x11,0x0B,0x0A,0x00,0x12,0x0F,0x13,0x03,0x18,0x1A,0x07,0x0C,0x04,0x08,
	0x0D,0x14,0x05,0x02,0x06,0x09,0x10,0x1D,0x15,0x16,0x1B,0xFD,0xFD,0xFD,0xFD,0xFD,
	0xFD,0x01,0x11,0x0B,0x0A,0x00,0x12,0x0F,0x13,0x03,0x18,0x1A,0x07,0x0C,0x04,0x08,
	0x0D,0x14,0x05,0x02,0x06,0x09,0x10,0x1D,0x15,0x16,0x1B,0xFD,0xFD,0xFD,0xFD,0xFE,
	0xFD,0xFF,0xFD,0x46,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0x31,0xFD,0x26,0xFF,0x47,0xFF,
	0xFF,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0x31,0xFD,0x26,0xFF,0x48,0x49,
	0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,
	0xFD,0xFD,0xFD,0xFD,0xFD,0x4A,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,
	0x19,0x27,0x21,0x25,0x2E,0x4B,0x33,0x1E,0x17,0x0E,0x1C,0x2C,0x2F,0x28,0x20,0x23,
	0x4C,0x32,0x34,0x29,0x1F,0x4D,0x2A,0xFD,0x4E,0x24,0x2D,0x22,0x2B,0x4F,0x50,0x30,
	0x19,0x27,0x21,0x25,0x2E,0x51,0x33,0x1E,0x17,0x0E,0x1C,0x2C,0x2F,0x28,0x20,0x23,
	0x52,0x32,0x34,0x29,0x1F,0x53,0x2A,0xFD,0x54,0x24,0x2D,0x22,0x2B,0x55,0x56,0x57
};

static const uint8_t ISO_8859_1_FRENCH_CHAR_TO_ORDER_MAP[] = {
	0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFC,0xFE,0xFE,0xFC,0xFE,0xFE,
	0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,
	0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,
	0xFB,0xFB,0xFB,0xFB,0xFB,0xFB,0xFB,0xFB,0xFB,0xFB,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,
	0xFD,0x01,0x11,0x0B,0x0A,0x00,0x12,0x0F,0x13,0x03,0x18,0x1A,0x07,0x0C,0x04,0x08,
	0x0D,0x14,0x05,0x02,0x06,0x09,0x10,0x1D,0x15,0x16,0x1B,0xFD,0xFD,0xFD,0xFD,0xFD,
	0xFD,0x01,0x11,0x0B,0x0A,0x00,0x12,0x0F,0x13,0x03,0x18,0x1A,0x07,0x0C,0x04,0x08,
	0x0D,0x14,0x05,0x02,0x06,0x09,0x10,0x1D,0x15,0x16,0x1B,0xFD,0xFD,0xFD,0xFD,0xFE,
	0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,
	0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,
	0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,
	0xFD,0xFD,0xFD,0xFD,0xFD,0x58,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,
	0x19,0x27,0x21,0x25,0x2E,0x59,0x33,0x1E,0x17,0x0E,0x1C,0x2C,0x2F,0x28,0x20,0x23,
	0x5A,0x32,0x34,0x29,0x1F,0x5B,0x2A,0xFD,0x5C,0x24,0x2D,0x22,0x2B,0x5D,0x5E,0x30,
	0x19,0x27,0x21,0x25,0x2E,0x5F,0x33,0x1E,0x17,0x0E,0x1C,0x2C,0x2F,0x28,0x20,0x23,
	0x60,0x32,0x34,0x29,0x1F,0x61,0x2A,0xFD,0x62,0x24,0x2D,0x22,0x2B,0x63,0x64,0x65
};

static const uint8_t ISO_8859_15_FRENCH_CHAR_TO_ORDER_MAP[] = {
	0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFC,0xFE,0xFE,0xFC,0xFE,0xFE,
	0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,
	0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,
	0xFB,0xFB,0xFB,0xFB,0xFB,0xFB,0xFB,0xFB,0xFB,0xFB,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,
	0xFD,0x01,0x11,0x0B,0x0A,0x00,0x12,0x0F,0x13,0x03,0x18,0x1A,0x07,0x0C,0x04,0x08,
	0x0D,0x14,0x05,0x02,0x06,0x09,0x10,0x1D,0x15,0x16,0x1B,0xFD,0xFD,0xFD,0xFD,0xFD,
	0xFD,0x01,0x11,0x0B,0x0A,0x00,0x12,0x0F,0x13,0x03,0x18,0x1A,0x07,0x0C,0x04,0x08,
	0x0D,0x14,0x05,0x02,0x06,0x09,0x10,0x1D,0x15,0x16,0x1B,0xFD,0xFD,0xFD,0xFD,0xFE,
	0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,
	0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,
	0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0x31,0xFD,0x31,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,
	0xFD,0xFD,0xFD,0xFD,0x35,0x36,0xFD,0xFD,0x37,0xFD,0xFD,0xFD,0x26,0x26,0x38,0xFD,
	0x19,0x27,0x21,0x25,0x2E,0x39,0x33,0x1E,0x17,0x0E,0x1C,0x2C,0x2F,0x28,0x20,0x23,
	0x3A,0x32,0x34,0x29,0x1F,0x3B,0x2A,0xFD,0x3C,0x24,0x2D,0x22,0x2B,0x3D,0x3E,0x30,
	0x19,0x27,0x21,0x25,0x2E,0x3F,0x33,0x1E,0x17,0x0E,0x1C,0x2C,0x2F,0x28,0x20,0x23,
	0x40,0x32,0x34,0x29,0x1F,0x41,0x2A,0xFD,0x42,0x24,0x2D,0x22,0x2B,0x43,0x44,0x45
};

/* Model Table:
 * Total sequences: 955
 * First 512 sequences: 0.9964629873582833
 * Next 512 sequences (512-1024): 0.0035370126417167387
 * Rest: -5.2909066017292616e-17
 * Negative sequences: TODO
 */
static const uint8_t FRENCH_LANG_MODEL[] = {
	3,3,3,3,3,3,3,3,3,3,3,3,3,3,0,3,3,3,3,3,3,3,3,0,3,0,3,3,0,3,3,0,0,0,2,2,0,0,0,
	3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,3,0,3,3,0,3,3,2,3,2,0,3,0,0,0,
	3,3,3,3,3,3,3,3,3,3,2,3,3,3,3,2,2,3,3,3,3,0,3,3,0,0,3,2,0,3,0,0,0,0,2,0,0,3,2,
	3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,0,3,3,2,2,2,2,0,0,0,0,0,2,0,
	3,3,3,3,3,3,3,3,3,3,3,3,3,2,3,3,3,3,3,3,3,2,3,3,3,0,3,3,2,2,3,0,2,2,0,0,0,2,2,
	3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,3,3,2,0,3,3,3,2,3,3,2,3,2,0,0,2,0,
	3,3,3,3,2,3,3,3,3,3,2,3,3,2,3,2,2,3,2,3,0,0,3,3,2,2,2,3,3,2,0,3,0,2,0,0,0,2,0,
	3,3,3,3,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,0,3,3,2,3,3,2,0,2,0,2,0,0,0,0,0,0,0,
	3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,0,3,3,0,3,2,2,2,0,3,3,3,2,0,
	3,3,3,3,3,3,3,3,3,2,3,3,3,3,3,3,3,3,3,3,2,3,3,3,3,0,3,3,3,2,2,0,0,0,0,2,0,0,0,
	3,3,3,3,3,3,2,2,3,3,3,2,3,2,3,3,3,2,2,3,0,0,3,3,3,0,2,2,0,3,0,2,2,0,2,0,0,0,0,
	3,3,3,3,2,3,3,3,3,3,3,3,2,0,3,2,2,2,2,3,3,2,3,3,2,2,3,2,2,0,0,3,0,2,0,0,0,0,2,
	3,3,3,3,3,2,2,2,3,3,2,3,3,3,3,2,2,3,2,2,0,2,3,3,0,0,2,2,3,2,0,2,0,2,2,0,0,2,2,
	3,3,3,3,0,3,3,3,3,3,2,3,2,3,3,2,2,2,2,3,0,0,2,3,0,0,3,2,2,0,2,3,0,2,0,0,0,0,0,
	3,3,3,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,0,2,3,0,2,2,0,0,2,0,0,2,0,2,0,0,0,
	3,3,3,3,3,3,3,3,3,3,2,2,3,2,3,3,2,2,2,3,0,2,3,3,0,2,2,2,2,2,0,0,0,2,0,0,0,0,0,
	3,3,2,3,0,3,2,2,3,3,2,2,0,2,3,2,0,2,2,0,0,0,2,3,0,0,0,0,3,0,0,2,0,0,0,0,0,0,0,
	3,3,3,3,3,3,3,3,3,3,3,3,2,2,3,2,2,3,0,2,0,0,3,2,3,0,2,0,2,0,0,0,2,3,2,0,0,0,2,
	3,3,3,3,2,3,3,3,3,3,0,2,3,2,3,3,0,2,3,0,0,2,2,3,0,0,0,0,3,0,0,0,0,0,2,0,0,0,0,
	3,3,3,3,3,3,3,3,3,3,2,2,3,2,3,0,0,2,0,2,0,0,3,3,2,0,2,2,2,2,0,3,0,3,2,0,0,2,2,
	0,2,0,2,0,0,0,0,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,
	3,3,0,3,0,0,3,2,3,2,0,3,0,3,3,0,3,2,0,2,2,3,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	3,3,3,3,3,3,3,3,3,2,3,3,3,3,3,2,2,3,2,2,2,2,2,2,0,0,2,2,0,2,0,0,0,2,0,0,0,0,0,
	0,0,3,0,3,3,3,3,0,0,3,3,3,2,0,3,3,3,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	3,3,2,3,0,3,2,0,3,3,2,2,0,2,3,0,0,2,0,0,0,0,0,0,2,3,0,0,2,0,0,0,0,2,0,0,0,0,0,
	0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	3,3,3,3,2,3,2,3,3,3,0,2,3,2,3,2,2,2,2,3,0,2,2,0,2,0,3,2,0,2,0,0,0,0,0,0,0,0,0,
	3,3,0,3,2,2,2,2,3,3,0,0,2,0,2,2,0,2,0,2,2,0,3,0,0,0,2,3,0,2,0,0,0,0,0,0,0,0,0,
	0,0,2,0,2,0,3,2,0,0,0,2,3,0,0,0,2,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	3,3,3,3,2,2,0,2,3,2,2,2,2,2,0,0,0,2,2,2,0,0,2,0,0,0,2,0,0,2,0,0,0,0,0,0,0,0,0,
	0,3,0,0,0,0,0,2,3,3,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,
	0,0,2,2,3,0,3,3,0,0,0,0,2,2,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,3,0,3,3,0,0,0,2,2,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	2,0,0,0,3,0,3,2,0,0,2,3,2,2,0,3,0,2,0,3,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,3,3,2,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	2,2,2,0,2,2,2,2,0,0,2,2,2,0,0,2,0,2,0,0,2,0,0,0,0,0,2,0,0,2,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,2,0,0,0,0,0,3,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

#ifdef IO_DECLSPEC
IO_PUBLIC_SYMBOL const sequence_model*
#else
const sequence_model* IO_PUBLIC_SYMBOL
#endif // IO_DECLSPEC
win1252_french_sequence_model() noexcept
{
	static const sequence_model ret = {
		WIN1252_FRENCH_CHAR_TO_ORDER_MAP,
		FRENCH_LANG_MODEL,
		0.958115F,
		1252
	};
	return &ret;
}

#ifdef IO_DECLSPEC
IO_PUBLIC_SYMBOL const sequence_model*
#else
const sequence_model* IO_PUBLIC_SYMBOL
#endif // IO_DECLSPEC
iso_8859_1_french_sequence_model() noexcept
{
	static const sequence_model ret = {
		ISO_8859_1_FRENCH_CHAR_TO_ORDER_MAP,
		FRENCH_LANG_MODEL,
		0.958115F,
		28591
	};
	return &ret;
}

#ifdef IO_DECLSPEC
IO_PUBLIC_SYMBOL const sequence_model*
#else
const sequence_model* IO_PUBLIC_SYMBOL
#endif // IO_DECLSPEC
iso_8859_15_french_sequence_model() noexcept
{
	static const sequence_model ret = {
		ISO_8859_15_FRENCH_CHAR_TO_ORDER_MAP,
		FRENCH_LANG_MODEL,
		0.958115F,
		28605
	};
	return &ret;
}

}	// namespace detail

} // namespace io



