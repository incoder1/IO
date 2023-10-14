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
	constexpr const std::size_t EFECTIVE_CODESET_LEN = 63;
	uint8_t koi8r[EFECTIVE_CODESET_LEN];
	uint8_t ch = 192;
	for(std::size_t i=0; i < EFECTIVE_CODESET_LEN; i++) {
		koi8r[i] = ch++;
	}
	io::detail::s_prober kio8r_prober = io::detail::single_byte_prober::create(ec_, io::detail::koi8r_sequence_model(), false );
	ASSERT_FALSE(ec_);
	io::detail::prober::state_t actual;
	for(std::size_t i=0; i < 32; i++) {
	 	actual = kio8r_prober->handle_data(ec_,koi8r, EFECTIVE_CODESET_LEN);
	 	ASSERT_FALSE(ec_);
	}
	ASSERT_EQ(io::detail::prober::state_t::found, actual);
}

TEST_F(charset_detector_fixture, prober_should_detect_windows_1251)
{
	constexpr const std::size_t EFECTIVE_CODESET_LEN = 128;
	uint8_t cp1251[EFECTIVE_CODESET_LEN];
	uint8_t ch = 128;
	for(std::size_t i=0; i < EFECTIVE_CODESET_LEN; i++) {
		cp1251[i] = ch++;
	}
	io::detail::s_prober cp1251_prober = io::detail::single_byte_prober::create(ec_, io::detail::win1251_sequence_model(), false );
	ASSERT_FALSE(ec_);
	io::detail::prober::state_t actual;
	for(std::size_t i=0; i < 32; i++) {
	 	actual = cp1251_prober->handle_data(ec_,cp1251, EFECTIVE_CODESET_LEN);
	 	ASSERT_FALSE(ec_);
	}
	ASSERT_EQ(io::detail::prober::state_t::found, actual);
}

TEST_F(charset_detector_fixture, prober_should_detect_iso_8859_5)
{
	constexpr const std::size_t EFECTIVE_CODESET_LEN = 79;
	uint8_t iso_8859_5[EFECTIVE_CODESET_LEN];
	uint8_t ch = 161;
	for(std::size_t i=0; i < EFECTIVE_CODESET_LEN; i++) {
		iso_8859_5[i] = ch++;
	}
	io::detail::s_prober iso_8859_5_prober = io::detail::single_byte_prober::create(ec_, io::detail::iso_8859_5_sequence_model(), false );
	ASSERT_FALSE(ec_);
	io::detail::prober::state_t actual;
	for(std::size_t i=0; i < 32; i++) {
	 	actual = iso_8859_5_prober->handle_data(ec_,iso_8859_5, EFECTIVE_CODESET_LEN);
	 	ASSERT_FALSE(ec_);
	}
	ASSERT_EQ(io::detail::prober::state_t::found, actual);
}

TEST_F(charset_detector_fixture, prober_should_detect_windows_1253)
{
	constexpr const std::size_t EFECTIVE_CODESET_LEN = 61;
	uint8_t cp1253[EFECTIVE_CODESET_LEN];
	uint8_t ch = 193;
	for(std::size_t i=0; i < EFECTIVE_CODESET_LEN; i++) {
		cp1253[i] = ch++;
	}
	io::detail::s_prober cp1253_prober = io::detail::single_byte_prober::create(ec_, io::detail::win1253_sequence_model(), false );
	ASSERT_FALSE(ec_);
	io::detail::prober::state_t actual;
	for(std::size_t i=0; i < 32; i++) {
	 	actual = cp1253_prober->handle_data(ec_, cp1253, EFECTIVE_CODESET_LEN);
	 	ASSERT_FALSE(ec_);
	}
	ASSERT_EQ(io::detail::prober::state_t::found, actual);
}

TEST_F(charset_detector_fixture, prober_should_detect_iso_8859_7)
{
	constexpr const std::size_t EFECTIVE_CODESET_LEN = 61;
	uint8_t iso_8859_7[EFECTIVE_CODESET_LEN];
	uint8_t ch = 193;
	for(std::size_t i=0; i < EFECTIVE_CODESET_LEN; i++) {
		iso_8859_7[i] = ch++;
	}
	io::detail::s_prober iso_8859_7_prober = io::detail::single_byte_prober::create(ec_, io::detail::iso_8859_7_sequence_model(), false );
	ASSERT_FALSE(ec_);
	io::detail::prober::state_t actual;
	for(std::size_t i=0; i < 32; i++) {
	 	actual = iso_8859_7_prober->handle_data(ec_,iso_8859_7, EFECTIVE_CODESET_LEN);
	 	ASSERT_FALSE(ec_);
	}
	ASSERT_EQ(io::detail::prober::state_t::found, actual);
}
