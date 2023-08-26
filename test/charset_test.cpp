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

#include <charsets.hpp>

charset_fixture::charset_fixture():
	testing::Test()
{
}

TEST_F(charset_fixture, charset_for_name_test)
{
	auto actual = io::code_pages::for_name("UTF-8");
	ASSERT_TRUE( actual.first );
	ASSERT_EQ( io::code_pages::UTF_8, actual.second );
	actual = io::code_pages::for_name("utf-8");
	ASSERT_TRUE( actual.first );
	ASSERT_EQ( io::code_pages::UTF_8, actual.second );
}

TEST_F(charset_fixture, should_found_all_supported)
{
	std::array<std::string,35> supported = {{
			"UTF-8",
			"UTF-16LE",
			"UTF-16BE",
			"UTF-32LE",
			"UTF-32BE",
			"UTF-7",
			"ASCII",
			"ISO-8859-1",
			"ISO-8859-2",
			"ISO-8859-3",
			"ISO-8859-4",
			"ISO-8859-5",
			"ISO-8859-6",
			"ISO-8859-7",
			"ISO-8859-8",
			"ISO-8859-9",
			"ISO-8859-10",
			"ISO-8859-11",
			"ISO-8859-12",
			"ISO-8859-13",
			"ISO-8859-14",
			"ISO-8859-15",
			"ISO-8859-16",
			"KOI8-R",
			"KOI8-U",
			"KOI8-RU",
			"CP1250",
			"CP1251",
			"CP1252",
			"CP1253",
			"CP1254",
			"CP1255",
			"CP1256",
			"CP1257",
			"CP1258"
		}
	};

	for(auto name: supported) {
		auto actual = io::code_pages::for_name(name.data());
		ASSERT_TRUE(actual.first);
	}

}
