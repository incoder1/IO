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

#include <cmath>

charset_detector_fixture::charset_detector_fixture():
	testing::Test(),
	ec_()
{}

static const char* UTF8_STR = "abcdefghijklmnopqrstuvwxyzПривет!Hello!Χαιρετίσματα!World!ABCDEFGHIJKLMNOPQRSTUVWXYZგამარჯობა!こんにちは!您好!abcdefghijklmnopqrstuvwxyz";

TEST_F(charset_detector_fixture, prober_filter_without_english_letters)
{
	const char* EXPECTED = "Привет!!Χαιρετίσματα!!გამარჯობა!こんにちは!您好!";
	io::byte_buffer buff = io::detail::prober::filter_without_english_letters(ec_, reinterpret_cast<const uint8_t*>(UTF8_STR), io_strlen(UTF8_STR) );
	ASSERT_FALSE(ec_);
	ASSERT_STREQ( EXPECTED, buff.position().cdata() );
}

TEST_F(charset_detector_fixture, prober_filter_with_english_letters)
{
	const char* UTF8_STR = "0123456789,<Hello>,{Привет},[Χαιρετίσματα],||გამარჯობა||--こんにちは--,__您好__!";
	const char* EXPECTED = "HelloПриветΧαιρετίσματαგამარჯობაこんにちは您好";
	io::byte_buffer buff = io::detail::prober::filter_with_english_letters(ec_, reinterpret_cast<const uint8_t*>(UTF8_STR),  io_strlen(UTF8_STR) );
	ASSERT_FALSE(ec_);
	ASSERT_STREQ( EXPECTED, buff.position().cdata() );
}

TEST_F(charset_detector_fixture, latin1_prober_test)
{
	io::detail::s_prober prb = io::detail::latin1_prober::create(ec_);
	ASSERT_FALSE(ec_);
	const char* LATIN1_ONLY = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz,.!?-+=";
	auto actual = prb->handle_data(ec_, reinterpret_cast<const uint8_t*>(LATIN1_ONLY), io_strlen(LATIN1_ONLY) );
	ASSERT_FALSE(ec_);
	ASSERT_EQ(io::detail::prober::state_t::detecting, actual);
	ASSERT_FLOAT_EQ( 1.0F,  prb->confidence() );
	prb->reset();
	actual = prb->handle_data(ec_, reinterpret_cast<const uint8_t*>(UTF8_STR), io_strlen(UTF8_STR) );
	ASSERT_EQ(io::detail::prober::state_t::notme, actual);
	ASSERT_FLOAT_EQ( 0.01F,  prb->confidence() );
}

TEST_F(charset_detector_fixture, utf8_prober_long_str_test)
{
	io::detail::s_prober prb = io::detail::utf8_prober::create(ec_);
	ASSERT_FALSE(ec_);
	auto actual = prb->handle_data(ec_, reinterpret_cast<const uint8_t*>(UTF8_STR), io_strlen(UTF8_STR) );
	ASSERT_EQ(io::detail::prober::state_t::found, actual);
	ASSERT_FLOAT_EQ( 0.99F,  prb->confidence() );
}

TEST_F(charset_detector_fixture, utf8_prober_short_str_test)
{
	io::detail::s_prober prb = io::detail::utf8_prober::create(ec_);
	ASSERT_FALSE(ec_);
	const char* UTF8_SHORT = "ΑΒΓΔΕ";
	auto actual = prb->handle_data(ec_, reinterpret_cast<const uint8_t*>(UTF8_SHORT), io_strlen(UTF8_SHORT) );
	ASSERT_EQ(io::detail::prober::state_t::found, actual);
	ASSERT_TRUE( std::trunc( prb->confidence() * 100.0F) >= 95 );
}
