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
#include "stringpool.hpp"

namespace io {


// string_pool
s_string_pool string_pool::create(std::error_code& ec) noexcept
{
	string_pool* result = new (std::nothrow) string_pool();
	if(nullptr == result) {
		ec = std::make_error_code( std::errc::not_enough_memory );
		return s_string_pool();
	}
	return s_string_pool(result);
}

string_pool::string_pool() noexcept:
	object(),
	pool_()
{
}

string_pool::~string_pool() noexcept
{
}

const_string string_pool::load(const std::size_t hash, const char* s,const std::size_t size) noexcept
{
	auto it = pool_.emplace(std::piecewise_construct, std::forward_as_tuple(hash), std::forward_as_tuple(s, size));
	return it.first->second;
}

bool string_pool::find(const_string& str,const std::size_t hash, const char* s,const std::size_t size) noexcept
{
	bool ret = false;
	switch( pool_.count(hash) ) {
	case 0:
		break;
	case 1:
		str = pool_.at(hash);
		ret = true;
		break;
	default:
		auto range = pool_.equal_range(hash);
		auto it = std::find_if(range.first, range.second, [s,size] (const std::pair<std::size_t,const_string> & entry) {
			return entry.second.equal(s, size);
		} );
		ret = range.second != it;
		if(ret)
			str = it->second;
		break;
	}
	return ret;
}

const_string string_pool::find_or_load(const char* s,const std::size_t size) noexcept
{
	const_string ret;
	const std::size_t hash = io::hash_bytes(s,size);
	// search for a string in the pool
	if( !find(ret, hash, s, size) ) {
		ret = load(hash, s, size);
	}
	return ret;
}

const const_string string_pool::get(const char* s,const std::size_t size) noexcept
{
	const_string ret;
	if( size > 0 && nullptr != s && '\0' != *s) {
		// no problem on small string optimized strings
		// no any heap memory allocation happing for character array
		// i.e. string array is less then
		// sizeof of struct like { char* cstr; size_t len}
		// and wee don't need pooling
		ret = (size <= detail::SSO_MAX ) ? const_string(s, size) : find_or_load(s, size);
	}
	return  ret;
}

} // namespace io
