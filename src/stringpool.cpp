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

cached_string::cached_string(const cached_string& other) noexcept:
    data_( other.data_ )
{
    if(nullptr != data_)
        // increase reference count
        intrusive_add_ref(data_);
}


cached_string::cached_string(const char* str, std::size_t length) noexcept:
    data_(nullptr)
{
    assert(nullptr != str && length > 0);
    data_ = memory_traits::malloc_array<uint8_t>( sizeof(std::size_t) + length + 1 );
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
    if( nullptr != data_ ) {
        char *c;
        for(c = const_cast<char*>( data() ); io_isspace(*c); c++);
        return '\0' == *c;
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

string_pool::~string_pool() noexcept
{}

const cached_string string_pool::get(const char* s, std::size_t count) noexcept
{
    if( io_unlikely( (nullptr == s || '\0' == *s || count == 0 ) ) )
        return cached_string();

    const std::size_t str_hash = io::hash_bytes(s,count);
    pool_type::const_iterator it = pool_.find( str_hash );
    if( it != pool_.end() ) {
        std::size_t i = pool_.count( str_hash );
        if( io_unlikely(i > 1) ) {
            do {
                if( it->second.equal(s, count) )
                    break;
                ++it;
            }
            while( --i > 0 );
        }
        return it->second;
    }
#ifndef IO_NO_EXCEPTIONS
    try {
        std::pair<pool_type::iterator,bool> ret = pool_.emplace( str_hash, cached_string(s, count) );
        return ret.second ? ret.first->second : cached_string(s, count);
    }
    catch(...) {
        return cached_string(s, count);
    }
#else
    std::pair<pool_type::iterator,bool> ret = pool_.emplace( str_hash, cached_string(s, count) );
    return ret.second ? ret.first->second : cached_string(s, count);
#endif // IO_NO_EXCEPTIONS
}


} // namespace io
