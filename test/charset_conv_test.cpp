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

#include <io/core/memory_channel.hpp>

#include <io/textapi/charset_converter.hpp>

static const char* BYTE_1 = "a";
static const char* BYTE_2 = "α";
static const char* BYTE_3 = "ァ";
static const char* BYTE_4 = "𒀀";

static const char* UTF8_STR = "Hello!Привет!Χαιρετίσματα!გამარჯობა!こんにちは!您好!";
static const wchar_t* W_STR = L"Hello!Привет!Χαιρετίσματα!გამარჯობა!こんにちは!您好!";
static const char16_t* UTF16_STR = u"Hello!Привет!Χαιρετίσματα!გამარჯობა!こんにちは!您好!";
static const char32_t* UTF32_STR = U"Hello!Привет!Χαιρετίσματα!გამარჯობა!こんにちは!您好!";

charset_conv_fixture::charset_conv_fixture():
	testing::Test()
{
}

TEST_F(charset_conv_fixture, utf8_mutibibyte_symbol_len)
{
	ASSERT_EQ(1,io::utf8::mblen(BYTE_1));
	ASSERT_EQ(2,io::utf8::mblen(BYTE_2));
	ASSERT_EQ(3,io::utf8::mblen(BYTE_3));
	ASSERT_EQ(4,io::utf8::mblen(BYTE_4));
}

TEST_F(charset_conv_fixture, utf8_b1_symbol_decode_to_char32)
{
	char32_t actual;
	const char* shift = nullptr;
	shift = io::utf8::mbtochar32(actual,BYTE_1);
	ASSERT_EQ( (BYTE_1+1), shift);
	ASSERT_EQ( U'a', actual);
}

TEST_F(charset_conv_fixture, utf8_b2_symbol_decode_to_char32)
{
	char32_t actual;
	const char* shift = nullptr;
	shift = io::utf8::mbtochar32(actual,BYTE_2);
	ASSERT_EQ( (BYTE_2+2), shift);
	ASSERT_EQ( U'α', actual);
}

TEST_F(charset_conv_fixture, utf8_b3_symbol_decode_to_char32)
{
	char32_t actual;
	const char* shift = nullptr;
	shift = io::utf8::mbtochar32(actual,BYTE_3);
	ASSERT_EQ( (BYTE_3+3), shift);
	ASSERT_EQ( U'ァ', actual);
}

TEST_F(charset_conv_fixture, utf8_b4_symbol_decode_to_char32)
{
	char32_t actual;
	const char* shift = nullptr;
	shift = io::utf8::mbtochar32(actual,BYTE_4);
	ASSERT_EQ( (BYTE_4+4), shift);
	ASSERT_EQ( U'𒀀', actual);
}

TEST_F(charset_conv_fixture, utf8_multibyte_string_character_legth)
{
	const std::size_t EXPECTED = 45;
	ASSERT_EQ( EXPECTED, io::utf8::strlength(UTF8_STR) );
}

TEST_F(charset_conv_fixture, chars_utf8_to_utf16)
{
	char16_t actual[64] = {u'\0'};
	io::transcode(ec_, reinterpret_cast<const uint8_t*>(UTF8_STR), io_strlen(UTF8_STR), const_cast<char16_t*>(actual), 64);
	ASSERT_FALSE(ec_);
	ASSERT_EQ( 0, std::char_traits<char16_t>::compare(UTF16_STR, actual, std::char_traits<char16_t>::length(UTF16_STR) ) );
}

TEST_F(charset_conv_fixture, utf8_to_utf32)
{
	char32_t actual[64] = {U'\0'};
	io::transcode(ec_, reinterpret_cast<const uint8_t*>(UTF8_STR), io_strlen(UTF8_STR), const_cast<char32_t*>(actual), 64);
	ASSERT_FALSE(ec_);
	ASSERT_EQ( 0, std::char_traits<char32_t>::compare(UTF32_STR, actual, std::char_traits<char32_t>::length(UTF32_STR) ) );
}

TEST_F(charset_conv_fixture, chars_utf16_to_utf8)
{
	uint8_t actual[128] = {0};
	io::transcode(ec_, UTF16_STR,  std::char_traits<char16_t>::length(UTF16_STR), actual, 128);
	ASSERT_FALSE(ec_);
	ASSERT_STREQ( UTF8_STR, reinterpret_cast<char*>(actual) );
}

TEST_F(charset_conv_fixture, utf32_to_utf8)
{
	std::error_code ec;
	uint8_t actual[128] = {0};
	io::transcode(ec_, UTF32_STR,  std::char_traits<char32_t>::length(UTF32_STR), actual, 128);
	ASSERT_FALSE(ec_);
	ASSERT_STREQ( UTF8_STR, reinterpret_cast<char*>(actual) );
}

TEST_F(charset_conv_fixture, string_utf16_to_utf8)
{
	ASSERT_STREQ(UTF8_STR, io::transcode(UTF16_STR).data() );
}

TEST_F(charset_conv_fixture, string_utf32_to_utf8)
{
	ASSERT_STREQ(UTF8_STR, io::transcode(UTF32_STR).data() );
}

TEST_F(charset_conv_fixture, string_utf8_to_utf16)
{
	ASSERT_EQ( std::u16string(UTF16_STR), io::transcode_to_u16(UTF8_STR) );
}

TEST_F(charset_conv_fixture, string_utf8_to_utf32)
{
	ASSERT_EQ( std::u32string(UTF32_STR), io::transcode_to_u32(UTF8_STR).data() );
}

TEST_F(charset_conv_fixture, wchar_t_to_utf8)
{
	ASSERT_EQ(std::string(UTF8_STR), io::transcode(W_STR));
}

TEST_F(charset_conv_fixture, utf8_to_wchar_t)
{
	ASSERT_EQ(std::wstring(W_STR), io::transcode_to_ucs(UTF8_STR));
}

TEST_F(charset_conv_fixture, converting_channel_funnel_cp1251_to_uft8) {

    // GIVEN
	const uint8_t CP1251_SRC[6] = { 0xCF, 0xF0, 0xE8, 0xE2, 0xE5, 0xF2 };
	const char* EXPECTED_UTF8 = "Привет";

    io::s_memory_write_channel out = io::memory_write_channel::open(ec_);
    ASSERT_FALSE(ec_) << "Failed to open output memory channel" << ec_.message();

	io::s_funnel test_instance = io::charset_converting_channel_funnel::create(ec_,
				out,
				io::code_pages::cp1251(),
				io::code_pages::utf8(),
				32);

	// WHEN
	// Check we not in the initialization error
	ASSERT_FALSE(ec_) << "Failed to open charset converter funnel " << ec_.message();

    // Write via funnel (updating the fixture's ec)
    std::size_t bytes_written = test_instance->push(ec_, CP1251_SRC, sizeof(CP1251_SRC));
    EXPECT_GT(bytes_written, 0);
    ASSERT_FALSE(ec_) << "Write failed: " << ec_.message();
    test_instance->flush(ec_);
    ASSERT_FALSE(ec_) << "Flushing failed: " << ec_.message();

    // THEN
	io::byte_buffer written = out->data(ec_);
	ASSERT_FALSE(ec_) << "Take written memory failed: " << ec_.message();
    const char* actual = written.position().cdata();
    ASSERT_STREQ(actual, EXPECTED_UTF8);
}
