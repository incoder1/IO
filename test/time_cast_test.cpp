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

const char* time_cast_fixture::FORMAT = "%Y-%m-%dT%H:%M:%SZ";
const char* time_cast_fixture::EXPECTED_TIME_STR = "2022-09-28T22:35:00Z";

time_cast_fixture::time_cast_fixture():
	testing::Test(),
	ec_()
{}

TEST_F(time_cast_fixture, time_point_to_chars)
{
//	std::tm tm{};  // zero initialise
//	tm.tm_year = 2022-1900; // 2022
//	tm.tm_mon = 9-1; // September
//	tm.tm_mday = 28; // 28th
//	tm.tm_hour = 22;
//	tm.tm_min = 35;
//	tm.tm_isdst = 1;
//	std::time_t tp = std::mktime(&tm);
//
//	char actual[128] = {'\0'};
//	char* first = &actual[0];
//	char* last = first + sizeof(actual);
//	auto ret = io::to_chars(first, last, FORMAT, tp);
//	ec_ =  std::make_error_code(ret.ec);
//	ASSERT_FALSE( ec_ );
//	ASSERT_STREQ(EXPECTED_TIME_STR,actual);
}

TEST_F(time_cast_fixture, time_from_chars)
{
//	std::time_t parsed;
//	auto ret = io::from_chars(EXPECTED_TIME_STR, EXPECTED_TIME_STR+io_strlen(EXPECTED_TIME_STR), FORMAT, parsed);
//
//	ec_ =  std::make_error_code(ret.ec);
//	ASSERT_FALSE( ec_ );
//
//	// Convert sting back and check, to get time in GMT timezone
//	char actual[128] = {'\0'};
//	char* first = &actual[0];
//	char* last = first + sizeof(actual);
//	ret = io::to_chars(first, last, FORMAT,parsed);
//	ASSERT_FALSE( ec_ );
//	ASSERT_STREQ(EXPECTED_TIME_STR,actual);
}
