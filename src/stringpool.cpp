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
	return nullptr != result ? s_string_pool(result) : s_string_pool();
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
	typedef pool_type::value_type pair_type;
	if( io_unlikely( (nullptr == s || '\0' == *s || count == 0 ) ) )
		return const_string();

	// no problem on SSO string
	// no any heap memory allocation happing for character array
	// i.e. string array is less then
	// sizeof of struct like { char* cstr; size_t len}
	// and wee need pool nothing
 	if( count <= detail::SSO_MAX ) {
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
			it = std::find_if(its.first, its.second, [s,count] (const pair_type& entry) {
				return entry.second.equal(s, count);
			} );
		}
		return it->second;
	}
	// string is not found in hash table, construct and insert it
#ifndef IO_NO_EXCEPTIONS
	try {
#endif // IO_NO_EXCEPTIONS
		auto ret =
			pool_.emplace(
					std::piecewise_construct,
					std::forward_as_tuple(str_hash),
					std::forward_as_tuple(s, count)
				);
		return ret.first->second;
#ifndef IO_NO_EXCEPTIONS
	}
	catch(std::exception&) {
	  // seems like we are out of memory, return empty string
	   return const_string();
	}
#endif // IO_NO_EXCEPTIONS
}

} // namespace io
