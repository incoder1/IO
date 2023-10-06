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

#include <io/core/hashing.hpp>

#include <string>
#include <algorithm>

hash_fixture::hash_fixture():
	testing::Test()
{
}

static void check_sonsitancy_param(const char* str, std::size_t len) noexcept
{
	const std::size_t lhs_hash = io::hash_bytes(str,len);
	// Function return identical value when called more then once
	for(std::size_t i = 0; i < 4; i++) {
		ASSERT_EQ(lhs_hash, io::hash_bytes(str,len));
	}
	std::string reversed( (str), (str+len) );
	std::reverse(reversed.begin(), reversed.end());
	std::size_t rhs_hash = io::hash_bytes(reversed.data(), len);
	// Same length block should produce another hash
	ASSERT_NE(lhs_hash, rhs_hash);
}


TEST_F(hash_fixture, test_consistancy)
{
	const char* STR_LEN_4 = "0123";
	const char* STR_LEN_12 = "0123456789AB";
	const char* STR_LEN_16 = "0123456789ABCDEF";
	const char* STR_LEN_24 = "0123456789ABCDEFGHIJKLM";
	const char* STR_LEN_36 = "0123456789ABCDEFGHIJKLMNOPQRSTUVXYZ";

	check_sonsitancy_param(STR_LEN_4, 4);
	check_sonsitancy_param(STR_LEN_12, 12);
	check_sonsitancy_param(STR_LEN_16, 16);
	check_sonsitancy_param(STR_LEN_24, 24);
	check_sonsitancy_param(STR_LEN_36, 36);

	std::string str;
	str.reserve(128);
	str.append(STR_LEN_4);
	str.append(STR_LEN_12);
	str.append(STR_LEN_16);
	str.append(STR_LEN_24);
	str.append(STR_LEN_36);

	check_sonsitancy_param(str.data(), str.length());
}


TEST_F(hash_fixture, test_hash_combine_consistancy)
{
	struct Foo {
		std::size_t bar;
		std::size_t buz;
	};
	Foo foo;
	foo.bar = (std::numeric_limits<std::size_t>::min)();
	foo.buz = (std::numeric_limits<std::size_t>::max)();

	std::size_t lhs = 0;
	io::hash_combine(lhs, foo.bar);
	io::hash_combine(lhs, foo.buz);

	std::size_t rhs = 0;
	io::hash_combine(rhs, foo.bar);
	io::hash_combine(rhs, foo.buz);

	ASSERT_EQ(lhs, rhs);

	rhs = 0;
	io::hash_combine(rhs, foo.buz);
	io::hash_combine(rhs, foo.bar);
	ASSERT_NE(lhs, rhs);
}
