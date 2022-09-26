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
#include <char_cast.hpp>

string_cast_fixture::string_cast_fixture():
	testing::Test(),
	ec_()
{}

TEST_F(string_cast_fixture, unsingned_8bit_to_string)
{
	const uint8_t u8bit = 0xFF;
	auto actual = io::to_string(ec_, u8bit);
	ASSERT_FALSE( ec_ );
	ASSERT_STREQ("255", actual.data());
}

TEST_F(string_cast_fixture, unsingned_8bit_from_string)
{
	const char* src = "\t\n\v\f\r 255";
	const uint8_t expected = 0xFF;
	uint8_t actual;
	io::from_string(ec_, io::const_string(src), actual);
	ASSERT_FALSE( ec_ );
	ASSERT_EQ(expected, actual);
}

TEST_F(string_cast_fixture, unsingned_8bit_from_string_overflow)
{
	const char* src = "256";
	uint8_t actual;
	io::from_string(ec_, io::const_string(src), actual);
	ASSERT_TRUE( ec_ );
	ASSERT_EQ( std::errc::result_out_of_range , static_cast<std::errc>(ec_.value()) );
}

TEST_F(string_cast_fixture, unsingned_16bit_to_string)
{
	const uint16_t u16bit = 0xFFFF;
	auto actual = io::to_string(ec_, u16bit);
	ASSERT_FALSE( ec_ );
	ASSERT_STREQ("65535", actual.data());
}

TEST_F(string_cast_fixture, unsingned_16bit_from_string)
{
	const char* src = "\t\n\v\f\r 65535";
	const uint16_t expected = 0xFFFF;
	uint16_t actual;
	io::from_string(ec_, io::const_string(src), actual);
	ASSERT_FALSE( ec_ );
	ASSERT_EQ(expected, actual);
}

TEST_F(string_cast_fixture, unsingned_16bit_from_string_overflow)
{
	const char* src = "65536";
	uint16_t actual;
	io::from_string(ec_, io::const_string(src), actual);
	ASSERT_TRUE( ec_ );
	ASSERT_EQ( std::errc::result_out_of_range , static_cast<std::errc>(ec_.value()) );
}

TEST_F(string_cast_fixture, unsingned_32bit_to_string)
{
	const uint32_t u32bit = 0xFFFFFFFF;
	auto actual = io::to_string(ec_, u32bit);
	ASSERT_FALSE( ec_ );
	ASSERT_STREQ("4294967295", actual.data());
}

TEST_F(string_cast_fixture, unsingned_32bit_from_string_overflow)
{
	const char* src = "4294967296";
	uint32_t actual;
	io::from_string(ec_, io::const_string(src), actual);
	ASSERT_TRUE( ec_ );
	ASSERT_EQ( std::errc::result_out_of_range , static_cast<std::errc>(ec_.value()) );
}

TEST_F(string_cast_fixture, unsingned_32bit_from_string)
{
	const char* src = "\t\n\v\f\r 4294967295";
	const uint32_t expected = 0xFFFFFFFF;
	uint32_t actual;
	io::from_string(ec_, io::const_string(src), actual);
	ASSERT_FALSE( ec_ );
	ASSERT_EQ(expected, actual);
}

TEST_F(string_cast_fixture, unsingned_64bit_to_string)
{
	const uint64_t u64bit = 0xFFFFFFFFFFFFFFFF;
	auto actual = io::to_string(ec_, u64bit);
	ASSERT_FALSE( ec_ );
	ASSERT_STREQ("18446744073709551615", actual.data());
}

TEST_F(string_cast_fixture, unsingned_64bit_from_string)
{
	const char* src = "\t\n\v\f\r 18446744073709551615";
	const uint64_t expected =  0xFFFFFFFFFFFFFFFF;
	uint64_t actual;
	io::from_string(ec_, io::const_string(src), actual);
	ASSERT_FALSE( ec_ );
	ASSERT_EQ(expected, actual);
}

TEST_F(string_cast_fixture, singned_8bit_to_string)
{
	auto actual = io::to_string(ec_, -128);
	ASSERT_FALSE( ec_ );
	ASSERT_STREQ("-128", actual.data());

	actual = io::to_string(ec_, 127);
	ASSERT_FALSE( ec_ );
	ASSERT_STREQ("127", actual.data());
}

TEST_F(string_cast_fixture, singned_8bit_from_string)
{
	int8_t actual;
	io::from_string(ec_, "\t\n\v\f\r -128", actual);
	ASSERT_FALSE( ec_ );
	ASSERT_EQ(-128, actual);

	io::from_string(ec_, "\t\n\v\f\r 127", actual);
	ASSERT_FALSE( ec_ );
	ASSERT_EQ(127, actual);
}

TEST_F(string_cast_fixture, singned_8bit_from_string_overflow)
{
	int8_t actual;
	io::from_string(ec_, "-129", actual);
	ASSERT_TRUE( ec_ );
	ASSERT_EQ( std::errc::result_out_of_range , static_cast<std::errc>(ec_.value()) );

	io::from_string(ec_, "128", actual);
	ASSERT_TRUE( ec_ );
	ASSERT_EQ( std::errc::result_out_of_range , static_cast<std::errc>(ec_.value()) );
}

TEST_F(string_cast_fixture, singned_16bit_to_string)
{
	auto actual = io::to_string(ec_, -32768);
	ASSERT_FALSE( ec_ );
	ASSERT_STREQ("-32768", actual.data());

	actual = io::to_string(ec_, 32767);
	ASSERT_FALSE( ec_ );
	ASSERT_STREQ("32767", actual.data());
}


TEST_F(string_cast_fixture, singned_16bit_from_string)
{
	int16_t actual;
	io::from_string(ec_, "\t\n\v\f\r -32768", actual);
	ASSERT_FALSE( ec_ );
	ASSERT_EQ(-32768, actual);

	io::from_string(ec_, "-12345", actual);
	ASSERT_FALSE( ec_ );
	ASSERT_EQ(-12345, actual);

	io::from_string(ec_, "12345", actual);
	ASSERT_FALSE( ec_ );
	ASSERT_EQ(12345, actual);

	io::from_string(ec_, "\t\n\v\f\r 32767", actual);
	ASSERT_FALSE( ec_ );
	ASSERT_EQ(32767, actual);
}

TEST_F(string_cast_fixture, singned_16bit_from_string_overflow)
{
	int16_t actual;
	io::from_string(ec_, "-32769", actual);
	ASSERT_TRUE( ec_ );
	ASSERT_EQ( std::errc::result_out_of_range , static_cast<std::errc>(ec_.value()) );

	io::from_string(ec_, "32768", actual);
	ASSERT_TRUE( ec_ );
	ASSERT_EQ( std::errc::result_out_of_range , static_cast<std::errc>(ec_.value()) );
}

TEST_F(string_cast_fixture, singned_32bit_to_string)
{
	auto actual = io::to_string(ec_, -2147483648);
	ASSERT_FALSE( ec_ );
	ASSERT_STREQ("-2147483648", actual.data());

	actual = io::to_string(ec_, 2147483647);
	ASSERT_FALSE( ec_ );
	ASSERT_STREQ("2147483647", actual.data());
}


TEST_F(string_cast_fixture, singned_32bit_from_string)
{
	int32_t actual;
	io::from_string(ec_, "\t\n\v\f\r -2147483648", actual);
	ASSERT_FALSE( ec_ );
	ASSERT_EQ(-2147483648, actual);

	io::from_string(ec_, "-1234567890", actual);
	ASSERT_FALSE( ec_ );
	ASSERT_EQ(-1234567890, actual);

	io::from_string(ec_, "1234567890", actual);
	ASSERT_FALSE( ec_ );
	ASSERT_EQ(1234567890, actual);

	io::from_string(ec_, "\t\n\v\f\r 2147483647", actual);
	ASSERT_FALSE( ec_ );
	ASSERT_EQ(2147483647, actual);
}

TEST_F(string_cast_fixture, singned_32bit_from_string_overflow)
{
	int32_t actual;
	io::from_string(ec_, "-2147483649", actual);
	ASSERT_TRUE( ec_ );
	ASSERT_EQ( std::errc::result_out_of_range , static_cast<std::errc>(ec_.value()) );

	io::from_string(ec_, "2147483648", actual);
	ASSERT_TRUE( ec_ );
	ASSERT_EQ( std::errc::result_out_of_range , static_cast<std::errc>(ec_.value()) );
}

TEST_F(string_cast_fixture, singned_64bit_to_string)
{
	auto actual = io::to_string(ec_, LLONG_MIN);
	ASSERT_FALSE( ec_ );
	ASSERT_STREQ("-9223372036854775808", actual.data());

	actual = io::to_string(ec_, LLONG_MAX);
	ASSERT_FALSE( ec_ );
	ASSERT_STREQ("9223372036854775807", actual.data());
}


TEST_F(string_cast_fixture, singned_64bit_from_string)
{
	int64_t actual;
	io::from_string(ec_, "\t\n\v\f\r -9223372036854775808", actual);

	ASSERT_FALSE( ec_ );
	ASSERT_EQ(LLONG_MIN, actual);

	io::from_string(ec_, "-1234567890123456789", actual);
	ASSERT_FALSE( ec_ );
	ASSERT_EQ(-1234567890123456789L, actual);

	io::from_string(ec_, "1234567890123456789", actual);
	ASSERT_FALSE( ec_ );
	ASSERT_EQ(1234567890123456789L, actual);

	io::from_string(ec_, "\t\n\v\f\r 9223372036854775807", actual);
	ASSERT_FALSE( ec_ );
	ASSERT_EQ(LLONG_MAX, actual);
}

TEST_F(string_cast_fixture, float_to_string)
{
	const float f32 = -1.234567F;
	static const char* expected  = "-1.234567";
	auto actual = io::to_string(ec_, f32);
	ASSERT_FALSE( ec_ );
	ASSERT_STREQ(expected, actual.data());
}

TEST_F(string_cast_fixture, float_from_string)
{
	const float expected = -12345678.9F;
	float actual;
	io::from_string(ec_, "\t\n\v\f\r -12345678.9", actual);
	ASSERT_FALSE( ec_ );
	ASSERT_FLOAT_EQ(expected, actual);
}

TEST_F(string_cast_fixture, double_to_string)
{
	const double f64 = -1.234567890123456;
	static const char* expected  = "-1.234567890123456";
	auto actual = io::to_string(ec_, f64);
	ASSERT_FALSE( ec_ );
	ASSERT_STREQ(expected, actual.data());
}

TEST_F(string_cast_fixture, double_from_string)
{
	const double expected = -12345678.9;
	double actual;
	io::from_string(ec_, "\t\n\v\f\r -12345678.9", actual);
	ASSERT_FALSE( ec_ );
	ASSERT_DOUBLE_EQ(expected, actual);
}

TEST_F(string_cast_fixture, long_double_to_string)
{
	const long double f80 = -1.234567890123456789L;
	static const char* expected  = "-1.234567890123456789";

	auto actual = io::to_string(ec_, f80);
	ASSERT_FALSE( ec_ );
	ASSERT_STREQ(expected, actual.data());
}

TEST_F(string_cast_fixture, long_double_from_string)
{
	const long double expected = -12345678.9L;
	double actual;
	io::from_string(ec_, "\t\n\v\f\r -12345678.9", actual);
	ASSERT_FALSE( ec_ );
	// FIXME (Victor Gubin) [9/07/2022] : Implement full featured long double asserter
	ASSERT_DOUBLE_EQ(expected, actual);
}

TEST_F(string_cast_fixture, boolean_to_string_true_false)
{
	static const char* expected_positive = "true";

	auto actual = io::to_string(ec_, true);
	ASSERT_FALSE( ec_ );
	ASSERT_STREQ( expected_positive, actual.data() );

	static const char* expected_negative = "false";
	actual = io::to_string(ec_, false);
	ASSERT_FALSE( ec_ );
	ASSERT_STREQ(expected_negative, actual.data());
}

TEST_F(string_cast_fixture, boolean_to_string_yes_no)
{
	static const char* expected_positive = "yes";

	auto actual = io::to_string(true, io::str_bool_format::yes_no);
	ASSERT_STREQ( expected_positive, actual.data() );

	static const char* expected_negative = "no";
	actual = io::to_string(false, io::str_bool_format::yes_no);
	ASSERT_STREQ(expected_negative, actual.data());
}

TEST_F(string_cast_fixture,boolean_from_string)
{
	const char* src_true  = "\t\n\v\f\r True";
	const char* src_false  = "\t\n\v\f\r FaLsE";
	const char* src_yes  = "\t\n\v\f\r yEs";
	const char* src_no  = "\t\n\v\f\r No";

	bool actual;
	io::from_string(ec_, src_true, actual);
	ASSERT_FALSE( ec_ );
	ASSERT_TRUE( actual );

	io::from_string(ec_, src_false, actual);
	ASSERT_FALSE( ec_ );
	ASSERT_FALSE( actual );

	io::from_string(ec_, src_yes, actual);
	ASSERT_FALSE( ec_ );
	ASSERT_TRUE( actual );

	io::from_string(ec_, src_no, actual);
	ASSERT_FALSE( ec_ );
	ASSERT_FALSE( actual );
}

