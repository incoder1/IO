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
#include "io/core/stringpool.hpp"

namespace io {


// string_pool
s_string_pool string_pool::create(std::error_code& ec) noexcept
{
	s_string_pool ret;
#ifndef IO_NO_EXCEPTIONS
	try {
#endif // IO_NO_EXCEPTIONS
	string_pool* px = new (std::nothrow) string_pool( pool_type(64) );
	if(nullptr == px)
		ec = std::make_error_code( std::errc::not_enough_memory );
	else
		ret.reset(px, true);
#ifndef IO_NO_EXCEPTIONS
	} catch(std::bad_array_new_length& e) {
		ec = std::make_error_code(std::errc::not_enough_memory);
	}
#endif // IO_NO_EXCEPTIONS
	return ret;
}

string_pool::string_pool(pool_type&& pool) noexcept:
	object(),
	pool_( std::forward<pool_type>(pool) )
{
}

string_pool::~string_pool() noexcept
{
}

const_string string_pool::load(const std::size_t hash, const char* s,const std::size_t size) noexcept
{
#ifndef IO_NO_EXCEPTIONS
	try {
#endif // IO_NO_EXCEPTIONS
	auto it = pool_.emplace(std::piecewise_construct, std::forward_as_tuple(hash), std::forward_as_tuple(s, size));
	return it.first->second;
#ifndef IO_NO_EXCEPTIONS
	} catch(...) {
		return const_string(s, size);
	}
#endif // IO_NO_EXCEPTIONS
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
