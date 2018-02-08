/*
 *
 * Copyright (c) 2016
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

// cached_string
void cached_string::intrusive_add_ref(uint8_t* ptr) noexcept
{
	std::size_t volatile *p = reinterpret_cast<std::size_t volatile*>(ptr);
	detail::atomic_traits::inc(p);
}

bool cached_string::intrusive_release(uint8_t* ptr) noexcept
{
	std::size_t volatile *p = reinterpret_cast<std::size_t volatile*>(ptr);
	return static_cast<size_t>(0) == detail::atomic_traits::dec(p);
}


cached_string::cached_string(const cached_string& other) noexcept:
	data_( other.data_ )
{
	if(nullptr != data_)
		// increase reference count
		intrusive_add_ref(data_);
}

cached_string::cached_string(cached_string&& other) noexcept:
	data_(other.data_)
{
	other.data_ = nullptr;
}

cached_string::cached_string(const char* str, std::size_t length) noexcept:
	data_(nullptr)
{
	assert(nullptr != str && length > 0);
	const std::size_t len = sizeof(std::size_t) + length + 1;
	data_ = memory_traits::malloc_array<uint8_t>( len );
	if(nullptr != data_) {
		// set initial intrusive atomic reference count
		std::size_t *rc = reinterpret_cast<std::size_t*>(data_);
		*rc = 1;
		// copy string data
		char *b = reinterpret_cast<char*>( data_ + sizeof(std::size_t) );
		traits_type::copy(b, str, length);
	}
}

cached_string::~cached_string() noexcept
{
	if(nullptr != data_ && intrusive_release(data_) )
		memory_traits::free(data_);
}

bool cached_string::blank() const noexcept
{
	const char *c = data();
	while( *c != '\0') {
#ifdef io_isspace
		if( ! io_isspace( traits_type::to_int_type( *c ) ) )
#else
		if( !std::isspace( traits_type::to_int_type( *c ) ) )
#endif // io_isspace
			return false;
		++c;
	}
	return true;
}


// string_pool
s_string_pool string_pool::create(std::error_code& ec) noexcept
{
	string_pool* result = new (std::nothrow) string_pool();
	return nullptr != result ? s_string_pool(result) : s_string_pool();
}

string_pool::string_pool() noexcept:
	object(),
	pool_()
{}

#if __GNUG__
const cached_string string_pool::get(const char* s, std::size_t count) noexcept
{
	if( __builtin_expect( (nullptr == s || '\0' == *s || count == 0 ), 0) )
		return cached_string();
	const std::size_t str_hash = io::hash_bytes(s,count);
	pool_type::const_iterator i = pool_.find( str_hash );
	if(  i != pool_.end() ) {
		if( __builtin_expect( i->second.equal(s, count), 1 ) )
			return i->second;
		else
			return cached_string(s, count);
	}
	else {
		std::pair<pool_type::iterator,bool> ret = pool_.emplace( str_hash, cached_string(s, count) );
		if( __builtin_expect(ret.second,1) )
			return ret.first->second;
		else
			return cached_string(s, count);
	}
	return cached_string();
}

#else

const cached_string string_pool::get(const char* s, std::size_t count) noexcept
{
	if(nullptr == s || '\0' == *s || count == 0)
		return cached_string();
	const std::size_t str_hash = io::hash_bytes(s,count);
	pool_type::const_iterator i = pool_.find( str_hash );
	if( i == pool_.end() ) {
		auto ret = pool_.emplace( str_hash, cached_string(s, count) );
		return ret.second ? ret.first->second : cached_string(s, count);
	}
	else {
		return i->second.equal(s, count)
			   ? i->second : cached_string(s, count);
	}
	return cached_string();
}

#endif //  __GNUG__

} // namespace io
