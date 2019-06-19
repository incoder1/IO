/*
 * Copyright (c) 2016-2019
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#include "stdafx.hpp"
#include "unicode_bom.hpp"

namespace io {

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
