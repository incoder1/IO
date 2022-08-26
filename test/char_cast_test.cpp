/*
 *
 * Copyright (c) 2016-2022
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#include "stdafx.hpp"
#include "char_cast_test.hpp"

char_cast_fixture::char_cast_fixture():
	testing::Test()
{}

void char_cast_fixture::SetUp()
{
}

TEST_F(char_cast_fixture, unsingned_8bit_to_char)
{
	char buff[4] = {'\0'};
	char* first= buff;
	char* last = first + sizeof(buff);

	uint8_t u8bit = 128;
	io_zerro_mem(buff,sizeof(buff));
	auto ret = io::to_chars(first, last, u8bit);
	ASSERT_FALSE( std::make_error_code(ret.ec) );
	ASSERT_EQ( ret.ptr, (buff+3) );
	ASSERT_STREQ(first,"128");
}

TEST_F(char_cast_fixture, unsingned_16bit_to_char)
{
	char buff[8] = {'\0'};
	char* first= buff;
	char* last = first + sizeof(buff);

	uint16_t u16bit = 0xFFFF;
	io_zerro_mem(buff,sizeof(buff));
	auto ret = io::to_chars(first, last, u16bit);
	ASSERT_FALSE( std::make_error_code(ret.ec) );
	ASSERT_EQ( ret.ptr, (buff+5) );
	ASSERT_STREQ(first,"65535");
}

TEST_F(char_cast_fixture, unsingned_32bit_to_char)
{
	char buff[16] = {'\0'};
	char* first= buff;
	char* last = first + sizeof(buff);

	uint32_t u32bit = 0xFFFFFFFF;
	auto ret = io::to_chars(first, last, u32bit);
	ASSERT_FALSE( std::make_error_code(ret.ec) );
	ASSERT_EQ( ret.ptr, (buff+10) );
	ASSERT_STREQ(first,"4294967295");
}

TEST_F(char_cast_fixture, unsingned_64bit_to_char)
{
	char buff[32] = {'\0'};
	char* first= buff;
	char* last = first + sizeof(buff);

	uint64_t u64bit = 0xFFFFFFFFFFFFFFFF;
	auto ret = io::to_chars(first, last, u64bit);
	ASSERT_FALSE( std::make_error_code(ret.ec) );
	ASSERT_EQ( ret.ptr, (buff+20) );
	ASSERT_STREQ(first,"18446744073709551615");
}

TEST_F(char_cast_fixture,singned_8bit_to_char)
{
	char buff[8] = {'\0'};
	char* first= buff;
	char* last = first + sizeof(buff);

	int8_t s8bit = std::numeric_limits<int8_t>::min();
	auto ret = io::to_chars(first, last, s8bit);
	ASSERT_FALSE( std::make_error_code(ret.ec) );
	ASSERT_EQ( ret.ptr, (buff+4) );
	ASSERT_STREQ(first,"-128");

	s8bit = +127;
	io_zerro_mem(buff,sizeof(buff));
	ret = io::to_chars(first, last, s8bit);
	ASSERT_FALSE( std::make_error_code(ret.ec) );
	ASSERT_EQ( ret.ptr, (buff+3) );
	ASSERT_STREQ(first,"127");
}

TEST_F(char_cast_fixture,singned_16bit_to_char)
{
	char buff[16] = {'\0'};
	char* first= buff;
	char* last = first + sizeof(buff);

	int16_t s16bit = std::numeric_limits<int16_t>::min();
	auto ret = io::to_chars(first, last, s16bit);
	ASSERT_FALSE( std::make_error_code(ret.ec) );
	ASSERT_EQ( ret.ptr, (buff+6) );
	ASSERT_STREQ(first,"-32768");

	s16bit = std::numeric_limits<int16_t>::max();
	io_zerro_mem(buff,sizeof(buff));
	ret = io::to_chars(first, last, s16bit);
	ASSERT_FALSE( std::make_error_code(ret.ec) );
	ASSERT_EQ( ret.ptr, (buff+5) );
	ASSERT_STREQ(first,"32767");
}

TEST_F(char_cast_fixture,singned_32bit_to_char)
{
	char buff[32] = {'\0'};
	char* first= buff;
	char* last = first + sizeof(buff);

	int32_t s32bit = std::numeric_limits<int32_t>::min();

	auto ret = io::to_chars(first, last, s32bit);
	ASSERT_FALSE( std::make_error_code(ret.ec) );
	ASSERT_EQ( ret.ptr, (buff+11) );
	ASSERT_STREQ(first,"-2147483648");

	s32bit = std::numeric_limits<int32_t>::max();
	io_zerro_mem(buff,sizeof(buff));
	ret = io::to_chars(first, last, s32bit);
	ASSERT_FALSE( std::make_error_code(ret.ec) );
	ASSERT_EQ( ret.ptr, (buff+10) );
	ASSERT_STREQ(first,"2147483647");
}


TEST_F(char_cast_fixture,singned_64bit_to_char)
{
	char buff[32] = {'\0'};
	char* first= buff;
	char* last = first + sizeof(buff);

	int64_t s64bit = std::numeric_limits<int64_t>::min();
	auto ret = io::to_chars(first, last, s64bit);
	ASSERT_FALSE( std::make_error_code(ret.ec) );
	ASSERT_EQ( ret.ptr, (buff+20) );
	ASSERT_STREQ(first,"-9223372036854775808");

	s64bit = std::numeric_limits<int64_t>::max();
	io_zerro_mem(buff,sizeof(buff));
	ret = io::to_chars(first, last, s64bit);
	ASSERT_FALSE( std::make_error_code(ret.ec) );
	ASSERT_EQ( ret.ptr, (buff+19) );
	ASSERT_STREQ(first,"9223372036854775807");
}

TEST_F(char_cast_fixture,float_to_char)
{
	char buff[32] = {'\0'};
	char* first= buff;
	char* last = first + sizeof(buff);

	const float f32 = -123456.7F;
	static const char* expected  = "-1.234567e+005";
	auto ret = io::to_chars(first, last, f32);
	ASSERT_FALSE( std::make_error_code(ret.ec) );
	ASSERT_EQ( ret.ptr, first+io_strlen(expected) );
	ASSERT_STREQ(first,expected);
}

TEST_F(char_cast_fixture,double_to_char)
{
	char buff[32] = {'\0'};
	char* first= buff;
	char* last = first + sizeof(buff);

	const double f64 = -123456.789;
	static const char* expected  = "-1.234568e+005";
	auto ret = io::to_chars(first, last, f64);
	ASSERT_FALSE( std::make_error_code(ret.ec) );
	ASSERT_EQ( ret.ptr, first+io_strlen(expected) );
	ASSERT_STREQ(first,expected);
}


TEST_F(char_cast_fixture,long_double_to_char)
{
	char buff[32] = {'\0'};
	char* first= buff;
	char* last = first + sizeof(buff);

	const long double f80 = -123456.789L;
	static const char* expected  = "-1.234568e+05";
	auto ret = io::to_chars(first, last, f80);
	ASSERT_FALSE( std::make_error_code(ret.ec) );
	ASSERT_EQ( ret.ptr, first+io_strlen(expected) );
	ASSERT_STREQ(first,expected);
}

TEST_F(char_cast_fixture,boolean_to_chars_fromat_yes_no)
{
	char buff[8] = {'\0'};
	char* first= buff;
	char* last = first + sizeof(buff);
	static const char* expected_positive = "yes";

	auto ret = io::to_chars(first, last, true, io::str_bool_format::yes_no);
	ASSERT_FALSE( std::make_error_code(ret.ec) );
	ASSERT_EQ( ret.ptr, first+io_strlen(expected_positive) );
	ASSERT_STREQ(first,expected_positive);

	io_zerro_mem(buff,sizeof(buff));

	static const char* expected_negative = "no";
	ret = io::to_chars(first, last, false, io::str_bool_format::yes_no);
	ASSERT_FALSE( std::make_error_code(ret.ec) );
	ASSERT_EQ( ret.ptr, first+io_strlen(expected_negative) );
	ASSERT_STREQ(first,expected_negative);
}


TEST_F(char_cast_fixture,boolean_to_chars_fromat_true_false)
{
	char buff[8] = {'\0'};
	char* first= buff;
	char* last = first + sizeof(buff);
	static const char* expected_positive = "true";

	auto ret = io::to_chars(first, last, true);
	ASSERT_FALSE( std::make_error_code(ret.ec) );
	ASSERT_EQ( ret.ptr, first+io_strlen(expected_positive) );
	ASSERT_STREQ(first,expected_positive);

	io_zerro_mem(buff,sizeof(buff));

	static const char* expected_negative = "false";
	ret = io::to_chars(first, last, false);
	ASSERT_FALSE( std::make_error_code(ret.ec) );
	ASSERT_EQ( ret.ptr, first+io_strlen(expected_negative) );
	ASSERT_STREQ(first,expected_negative);
}


