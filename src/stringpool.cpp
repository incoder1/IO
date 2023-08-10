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

const const_string string_pool::get(const char* s, std::size_t count) noexcept
{
	if( io_unlikely( (nullptr == s || '\0' == *s || count == 0 ) ) )
		return const_string();

	// no problem on small string optimized strings
	// no any heap memory allocation happing for character array
	// i.e. string array is less then
	// sizeof of struct like { char* cstr; size_t len}
	// and wee don't need pooling
	if( io_likely(count <= detail::SSO_MAX) ) {
		return const_string(s, count);
	}

	// search for a string in the pool
	const std::size_t str_hash = io::hash_bytes(s,count);
	pool_type::iterator it = pool_.find( str_hash );
	if( it != pool_.end() ) {
		// handle unlikelly hash-miss collision,
		// if we have move then one strings in hash table with the same hash
		// CityHash used City32 for 32 bit instruction set, and City64 for 64 bit
		// to minimize hash collisions as much as possible
		if( io_unlikely( pool_.count( str_hash ) > 1 ) ) {
			auto its = pool_.equal_range( str_hash );
			// find string by comparing memory
			// more likely never going to happen
			it = std::find_if(its.first, its.second, [s,count] (const std::pair<std::size_t,const_string> & entry) {
				return entry.second.equal(s, count);
			} );
		}
		return it->second;
	}
	auto ret =
		pool_.emplace(
			std::piecewise_construct,
			std::forward_as_tuple(str_hash),
			std::forward_as_tuple(s, count)
		);
	return ret.first->second;
}

} // namespace io
