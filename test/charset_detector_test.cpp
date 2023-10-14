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

#include <io/textapi/charset_detector.hpp>
#include <io/core/console.hpp>

#include  "io/textapi/detail/latin1_prober.hpp"
#include  "io/textapi/detail/utf8_prober.hpp"
#include  "io/textapi/detail/single_byte_group_prober.hpp"

#include <cmath>

charset_detector_fixture::charset_detector_fixture():
	testing::Test(),
	ec_()
{}

TEST_F(charset_detector_fixture, prober_should_filter_without_english_letters)
{
	const char* UTF8_STR = "abcdefghijklmnopqrstuvwxyzПривет!Hello!Χαιρετίσματα!World!ABCDEFGHIJKLMNOPQRSTUVWXYZგამარჯობა!こんにちは!您好!abcdefghijklmnopqrstuvwxyz";
	const char* EXPECTED = "Привет!!Χαιρετίσματα!!გამარჯობა!こんにちは!您好!";
	io::byte_buffer buff = io::detail::prober::filter_without_english_letters(ec_, reinterpret_cast<const uint8_t*>(UTF8_STR), io_strlen(UTF8_STR) );
	ASSERT_FALSE(ec_);
	ASSERT_STREQ( EXPECTED, buff.position().cdata() );
}

TEST_F(charset_detector_fixture, prober_should_filter_with_english_letters)
{
	const char* UTF8_STR = "0123456789,<Hello>,{Привет},[Χαιρετίσματα],||გამარჯობა||--こんにちは--,__您好__!";
	const char* EXPECTED = "HelloПриветΧαιρετίσματαგამარჯობაこんにちは您好";
	io::byte_buffer buff = io::detail::prober::filter_with_english_letters(ec_, reinterpret_cast<const uint8_t*>(UTF8_STR),  io_strlen(UTF8_STR) );
	ASSERT_FALSE(ec_);
	ASSERT_STREQ( EXPECTED, buff.position().cdata() );
}

TEST_F(charset_detector_fixture, prober_should_detect_latin1_from_str_and_ignore_multibyte)
{
	io::detail::s_prober prb = io::detail::latin1_prober::create(ec_);
	ASSERT_FALSE(ec_);
	const char* LATIN1_ONLY = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz,.!?-+=";
	auto actual = prb->handle_data(ec_, reinterpret_cast<const uint8_t*>(LATIN1_ONLY), io_strlen(LATIN1_ONLY) );
	ASSERT_FALSE(ec_);
	ASSERT_EQ(io::detail::prober::state_t::detecting, actual);
	ASSERT_FLOAT_EQ( 1.0F,  prb->confidence() );
	prb->reset();
	const char* UTF8_STR = "Hello!Привет!Χαιρετίσματα!გამარჯობა!こんにちは!您好!";
	actual = prb->handle_data(ec_, reinterpret_cast<const uint8_t*>(UTF8_STR), io_strlen(UTF8_STR) );
	ASSERT_EQ(io::detail::prober::state_t::notme, actual);
	ASSERT_FLOAT_EQ( 0.01F,  prb->confidence() );
}

TEST_F(charset_detector_fixture, prober_should_detect_utf8_by_from_long_data_mixed_single_and_multibyte_characters)
{
	io::detail::s_prober prb = io::detail::utf8_prober::create(ec_);
	ASSERT_FALSE(ec_);
	const char* UTF8_STR_LONG = "abcdefghijklmnopqrstuvwxyzПривет!Hello!Χαιρετίσματα!World!ABCDEFGHIJKLMNOPQRSTUVWXYZგამარჯობა!こんにちは!您好!abcdefghijklmnopqrstuvwxyz";
	auto actual = prb->handle_data(ec_, reinterpret_cast<const uint8_t*>(UTF8_STR_LONG), io_strlen(UTF8_STR_LONG) );
	ASSERT_EQ(io::detail::prober::state_t::found, actual);
	ASSERT_FLOAT_EQ( 0.99F,  prb->confidence() );
}

TEST_F(charset_detector_fixture, prober_should_detect_utf8_by_short_data_multibyte_characters)
{
	io::detail::s_prober prb = io::detail::utf8_prober::create(ec_);
	ASSERT_FALSE(ec_);
	const char* UTF8_SHORT = "ΑΒΓΔΕ";
	auto actual = prb->handle_data(ec_, reinterpret_cast<const uint8_t*>(UTF8_SHORT), io_strlen(UTF8_SHORT) );
	ASSERT_FALSE(ec_);
	ASSERT_EQ(io::detail::prober::state_t::found, actual);
	ASSERT_TRUE( std::trunc( prb->confidence() * 100.0F) >= 95 );
}

TEST_F(charset_detector_fixture, prober_should_detect_koi8r)
{
	const uint8_t KOI8_R_STR[] = {
		0xC1,0xE1,0xC2,0xE2,0xD7,0xF7,0xC7,0xE7,0xC4,0xE4,0x20,0x2E,0xC5,0xE5,0xA3,0xB3,
		0xD6,0xF6,0xDA,0xFA,0x2C,0xE9,0x0A,0x0D,0xEA,0xC9,0xEB,0xCA,0xEC,0xCB,0xED,0xCC,
		0xEE,0xCD,0xEF,0xCE,0x2C,0xCF,0xF0,0x09,0xF2,0xD0,0x3F,0xD2,0xD3,0xF3,0xD4,0xF4,
		0xD5,0xF5,0xC6,0xE6,0xC8,0xE8,0xC3,0xE3,0xDE,0xFE,0x0D,0x25,0xFB,0x0A,0xFD,0xDB,
		0xFF,0xDD,0xF9,0xDF,0xF8,0xD9,0xFC,0xD8,0xE0,0xDC,0xF1,0xC0,0x21,0xD1
	};
	io::detail::s_prober kio8r_prober = io::detail::single_byte_prober::create(ec_, io::detail::koi8r_sequence_model(), false );
	ASSERT_FALSE(ec_);
	io::detail::prober::state_t actual;
	for(std::size_t i=0; i < 20; i++) {
	 	actual = kio8r_prober->handle_data(ec_,KOI8_R_STR, sizeof(KOI8_R_STR));
	 	ASSERT_FALSE(ec_);
	}
	ASSERT_EQ(io::detail::prober::state_t::found, actual);
}

TEST_F(charset_detector_fixture, prober_should_detect_windows_1251)
{
	const uint8_t CP1251_STR[] = {
		0xE0,0xC0,0xE1,0xC1,0xE2,0xC2,0xE3,0xC3,
		0xE4,0xC4,0x20,0x2E,0xE5,0xC5,0xB8,0xA8,
		0xE6,0xC6,0xE7,0xC7,0x2C,0xC8,0x0A,0x0D,
		0xC9,0xE8,0xCA,0xE9,0xCB,0xEA,0xCC,0xEB,
		0xCD,0xEC,0xCE,0xED,0x2C,0xEE,0xCF,0x09,
		0xD0,0xEF,0x3F,0xF0,0xF1,0xD1,0xF2,0xD2,
		0xF3,0xD3,0xF4,0xD4,0xF5,0xD5,0xF6,0xD6,
		0xF7,0xD7,0x0D,0x25,0xD8,0x0A,0xD9,0xF8,
		0xDA,0xF9,0xDB,0xFA,0xDC,0xFB,0xDD,0xFC,
		0xDE,0xFD,0xDF,0xFE,0x21,0xFF
	};
	io::detail::s_prober cp1251_prober = io::detail::single_byte_prober::create(ec_, io::detail::win1251_sequence_model(), false );
	ASSERT_FALSE(ec_);
	io::detail::prober::state_t actual;
	for(std::size_t i=0; i < 20; i++) {
	 	actual = cp1251_prober->handle_data(ec_,CP1251_STR, sizeof(CP1251_STR));
	 	ASSERT_FALSE(ec_);
	}
	ASSERT_EQ(io::detail::prober::state_t::found, actual);
}
