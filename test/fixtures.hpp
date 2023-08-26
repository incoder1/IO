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
#ifndef __IO_TEST_FIXTURES_HPP_INCLUDED__
#define __IO_TEST_FIXTURES_HPP_INCLUDED__

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>

#include <system_error>

class char_cast_fixture: public testing::Test
{
public:
	char_cast_fixture();
};

class string_cast_fixture: public testing::Test
{
public:
	string_cast_fixture();
protected:
	std::error_code ec_;
};

class time_cast_fixture: public testing::Test
{
public:
	time_cast_fixture();
	static const char* FORMAT;
	static const char* EXPECTED_TIME_STR;
protected:
	std::error_code ec_;
};

class charset_conv_fixture: public testing::Test
{
public:
	charset_conv_fixture();
};

class hash_fixture: public testing::Test
{
public:
	hash_fixture();
};

class charset_fixture: public testing::Test
{
public:
	charset_fixture();
};

#endif // __IO_TEST_FIXTURES_HPP_INCLUDED__
