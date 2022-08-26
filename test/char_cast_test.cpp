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

TEST_F(char_cast_fixture, unsingned_8bit_from_char)
{
	const char* src = "128";
	const char* endp = src + io_strlen(src);
	const uint8_t expected = 128;
	uint8_t actual;
	auto ret = io::from_chars(src, endp, actual );
	ASSERT_EQ(actual, expected);
	ASSERT_FALSE( std::make_error_code(ret.ec) );
	ASSERT_EQ( ret.ptr, endp );
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

TEST_F(char_cast_fixture, unsingned_16bit_from_char)
{
	const char* src = "65535";
	const char* endp = src + io_strlen(src);
	const uint16_t expected = 0xFFFF;
	uint16_t actual;
	auto ret = io::from_chars(src, endp, actual );
	ASSERT_EQ(actual, expected);
	ASSERT_FALSE( std::make_error_code(ret.ec) );
	ASSERT_EQ( ret.ptr, endp );
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


TEST_F(char_cast_fixture, unsingned_32bit_from_char)
{
	const char* src = "4294967295";
	const char* endp = src + io_strlen(src);
	const uint32_t expected = 0xFFFFFFFF;
	uint32_t actual;
	auto ret = io::from_chars(src, endp, actual );
	ASSERT_EQ(actual, expected);
	ASSERT_FALSE( std::make_error_code(ret.ec) );
	ASSERT_EQ( ret.ptr, endp );
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

TEST_F(char_cast_fixture, unsingned_64bit_from_char)
{
	const char* src = "18446744073709551615";
	const char* endp = src + io_strlen(src);
	const uint64_t expected = 0xFFFFFFFFFFFFFFFF;
	uint64_t actual;
	auto ret = io::from_chars(src, endp, actual );
	ASSERT_EQ(actual, expected);
	ASSERT_FALSE( std::make_error_code(ret.ec) );
	ASSERT_EQ( ret.ptr, endp );
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

TEST_F(char_cast_fixture, singned_8bit_from_char)
{
	const char* src_min = "-128";
	const char* endp_min = src_min + io_strlen(src_min);
	const int8_t expected_min = -128;

	int8_t actual;

	auto ret = io::from_chars(src_min, endp_min, actual );
	ASSERT_EQ(actual, expected_min);
	ASSERT_FALSE( std::make_error_code(ret.ec) );
	ASSERT_EQ( ret.ptr, endp_min );

	const char* src_max = "127";
	const char* endp_max = src_max + io_strlen(src_max);
	const uint8_t expected_max = 127;

	ret = io::from_chars(src_max, endp_max, actual );
	ASSERT_EQ(actual, expected_max );
	ASSERT_FALSE( std::make_error_code(ret.ec) );
	ASSERT_EQ( ret.ptr, endp_max );
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

TEST_F(char_cast_fixture, singned_16bit_from_char)
{
	const char* src_min = "-32768";
	const char* endp_min = src_min + io_strlen(src_min);
	const int16_t expected_min = -32768;

	int16_t actual;

	auto ret = io::from_chars(src_min, endp_min, actual );
	ASSERT_EQ(actual, expected_min);
	ASSERT_FALSE( std::make_error_code(ret.ec) );
	ASSERT_EQ( ret.ptr, endp_min );

	const char* src_max = "32767";
	const char* endp_max = src_max + io_strlen(src_max);
	int16_t expected_max = 32767;

	ret = io::from_chars(src_max, endp_max, actual );
	ASSERT_EQ(actual, expected_max );
	ASSERT_FALSE( std::make_error_code(ret.ec) );
	ASSERT_EQ( ret.ptr, endp_max );
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

TEST_F(char_cast_fixture, singned_32bit_from_char)
{
	const char* src_min = "-2147483648";
	const char* endp_min = src_min + io_strlen(src_min);
	const int32_t expected_min = -2147483648;

	int32_t actual;

	auto ret = io::from_chars(src_min, endp_min, actual );
	ASSERT_EQ(actual, expected_min);
	ASSERT_FALSE( std::make_error_code(ret.ec) );
	ASSERT_EQ( ret.ptr, endp_min );

	const char* src_max = "2147483647";
	const char* endp_max = src_max + io_strlen(src_max);
	int32_t expected_max = 2147483647;

	ret = io::from_chars(src_max, endp_max, actual );
	ASSERT_EQ(actual, expected_max );
	ASSERT_FALSE( std::make_error_code(ret.ec) );
	ASSERT_EQ( ret.ptr, endp_max );
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

TEST_F(char_cast_fixture, singned_64bit_from_char)
{
	const char* src_min = "-9223372036854775808";
	const char* endp_min = src_min + io_strlen(src_min);
	const int64_t expected_min = std::numeric_limits<int64_t>::min();

	int64_t actual;

	auto ret = io::from_chars(src_min, endp_min, actual );
	ASSERT_EQ(actual, expected_min);
	ASSERT_FALSE( std::make_error_code(ret.ec) );
	ASSERT_EQ( ret.ptr, endp_min );

	const char* src_max = "9223372036854775807";
	const char* endp_max = src_max + io_strlen(src_max);
	int64_t expected_max = std::numeric_limits<int64_t>::max();

	ret = io::from_chars(src_max, endp_max, actual );
	ASSERT_EQ(actual, expected_max );
	ASSERT_FALSE( std::make_error_code(ret.ec) );
	ASSERT_EQ( ret.ptr, endp_max );
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

TEST_F(char_cast_fixture,float_from_char)
{
	const char* src  = "\r\n\t\v -12345678.9";
	const char* endp  = src + io_strlen(src);

	const float expected = -12345678.9F;

	float actual;
	auto ret = io::from_chars(src, endp, actual);

	ASSERT_FALSE( std::make_error_code(ret.ec) );
	ASSERT_EQ( ret.ptr, endp );
	ASSERT_FLOAT_EQ(expected,actual);
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


TEST_F(char_cast_fixture,double_from_char)
{
	const char* src  = "\r\n\t\v -12345678.9";
	const char* endp  = src + io_strlen(src);

	const double expected = -12345678.9;

	double actual;
	auto ret = io::from_chars(src, endp, actual);

	ASSERT_FALSE( std::make_error_code(ret.ec) );
	ASSERT_EQ( ret.ptr, endp );
	ASSERT_DOUBLE_EQ(expected,actual);
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

TEST_F(char_cast_fixture,long_double_from_char)
{
	const char* src  = "\r\n\t\v -12345678.9";
	const char* endp  = src + io_strlen(src);

	const long double expected = -12345678.9L;

	double actual;
	auto ret = io::from_chars(src, endp, actual);
	ASSERT_FALSE( std::make_error_code(ret.ec) );
	ASSERT_EQ( ret.ptr, endp );
	// FIXME (Victor Gubin) [8/26/2022] : Implement full featured long double asserter
	ASSERT_DOUBLE_EQ(expected,actual);
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

TEST_F(char_cast_fixture,boolean_from_chars)
{
	const char* src_true  = "\r\n\t\v True";
	const char* endp_true  = src_true + io_strlen(src_true);

	bool actual;
	auto ret = io::from_chars(src_true, endp_true, actual);

	ASSERT_FALSE( std::make_error_code(ret.ec) );
	ASSERT_EQ( ret.ptr, endp_true );
	ASSERT_TRUE(actual);

	const char* src_false  = "\r\n\t\v FaLsE";
	const char* endp_false  = src_false + io_strlen(src_false);

	ret = io::from_chars(src_false, endp_false, actual);
	ASSERT_FALSE( std::make_error_code(ret.ec) );
	ASSERT_EQ( ret.ptr, endp_false );
	ASSERT_FALSE(actual);

	const char* src_yes  = "\r\n\t\v yEs";
	const char* endp_yes  = src_yes + io_strlen(src_yes);
	ret = io::from_chars(src_yes, endp_yes, actual);

	ASSERT_FALSE( std::make_error_code(ret.ec) );
	ASSERT_EQ( ret.ptr, endp_yes );
	ASSERT_TRUE(actual);

	const char* src_no  = "\r\n\t\v nO";
	const char* endp_no  = src_no + io_strlen(src_no);

	ret = io::from_chars(src_no, endp_no, actual);
	ASSERT_FALSE( std::make_error_code(ret.ec) );
	ASSERT_EQ( ret.ptr, endp_no );
	ASSERT_FALSE(actual);
}
