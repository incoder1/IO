#include "stdafx.hpp"


static const char* BYTE_1 = "a";
static const char* BYTE_2 = "α";
static const char* BYTE_3 = "ァ";
static const char* BYTE_4 = "𒀀";

const char* UTF8_STR = "Hello!Привет!Χαιρετίσματα!გამარჯობა!こんにちは!您好!";
const char16_t* UTF16_STR = u"Hello!Привет!Χαιρετίσματα!გამარჯობა!こんにちは!您好!";
const char32_t* UTF32_STR = U"Hello!Привет!Χαιρετίσματα!გამარჯობა!こんにちは!您好!";

charset_conv_fixture::charset_conv_fixture():
	testing::Test()
{}

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

TEST_F(charset_conv_fixture, utf8_to_utf16)
{
	std::error_code ec;
	char16_t actual[64] = {u'\0'};
	io::transcode(ec, reinterpret_cast<const uint8_t*>(UTF8_STR), io_strlen(UTF8_STR), const_cast<char16_t*>(actual), 64);
	ASSERT_FALSE(ec);
	ASSERT_EQ( 0, std::char_traits<char16_t>::compare(UTF16_STR, actual, std::char_traits<char16_t>::length(UTF16_STR) ) );
}

TEST_F(charset_conv_fixture, utf8_to_utf32)
{
	std::error_code ec;
	char32_t actual[64] = {U'\0'};
	io::transcode(ec, reinterpret_cast<const uint8_t*>(UTF8_STR), io_strlen(UTF8_STR), const_cast<char32_t*>(actual), 64);
	ASSERT_FALSE(ec);
	ASSERT_EQ( 0, std::char_traits<char32_t>::compare(UTF32_STR, actual, std::char_traits<char32_t>::length(UTF32_STR) ) );
}


TEST_F(charset_conv_fixture, utf16_to_utf8)
{
	std::error_code ec;
	uint8_t actual[128] = {0};
	io::transcode(ec, UTF16_STR,  std::char_traits<char16_t>::length(UTF16_STR), actual, 128);
	ASSERT_FALSE(ec);
	ASSERT_STREQ( UTF8_STR, reinterpret_cast<char*>(actual) );
}

TEST_F(charset_conv_fixture, utf32_to_utf8)
{
	std::error_code ec;
	uint8_t actual[128] = {0};
	io::transcode(ec, UTF32_STR,  std::char_traits<char32_t>::length(UTF32_STR), actual, 128);
	ASSERT_FALSE(ec);
	ASSERT_STREQ( UTF8_STR, reinterpret_cast<char*>(actual) );
}
