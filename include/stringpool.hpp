/*
 * Copyright (c) 2016
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef __IO_STRINGPOOL_HPP_INCLUDED__
#define __IO_STRINGPOOL_HPP_INCLUDED__

#include "config.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include "text.hpp"

#include <atomic>
#include <ostream>
#include <string>
#include <unordered_map>

#include "hashing.hpp"
#include "object.hpp"

namespace io {

typedef const_string cached_string;

class string_pool;
DECLARE_IPTR(string_pool);

/// \brief A pool of raw character arrays i.e. C style strings,
/*!
* Allocates memory for the binary equal string only once
* This can be used to save memory for storing string similar string objects
*/
class IO_PUBLIC_SYMBOL string_pool:public object
{
	string_pool(const string_pool&) = delete;
	string_pool& operator=(const string_pool&) = delete;
private:
	friend class nobadalloc<string_pool>;
	inline friend void intrusive_ptr_add_ref(string_pool* const obj) noexcept {
		intrusive_ptr_add_ref( static_cast<object* const>(obj) );
    }
    inline friend void intrusive_ptr_release(string_pool* const obj) noexcept {
    	intrusive_ptr_release( static_cast<object* const>(obj) );
    }
	/// Construct new string pool
	string_pool() noexcept;
public:

	static s_string_pool create(std::error_code& ec) noexcept {
		string_pool* result = nobadalloc<string_pool>::construct(ec);
		return nullptr != result ? s_string_pool(result) : s_string_pool();
	}

	virtual ~string_pool() noexcept override = default;

	/// Returns a cached_string object for the raw character array.
	/// Allocates memory, if string is not presented in this pool
	/// \param s source character array
	/// \param size size of array in bytes
	/// \return cached_string object or empty cached string if out of memory or size is 0
	/// \throw never throws
	const cached_string get(const char* s, std::size_t size) noexcept;
	/// Returns a cached_string object for the C zero ending string
	/// Allocates memory, if string is not presented in memory.
	/// \param s source zero terminated C string
	/// \return cached_string object or empty cached string if out of memory or s is "" or nullptr
	/// \throw never throws
	inline const cached_string get(const char* s) noexcept {
		return get(s, std::char_traits<char>::length(s) );
	}
	/// Returns count of strings cached by this pool
	/// \return count of strings
	inline std::size_t size() const {
		return pool_.size();
	}
private:
	typedef std::unordered_map<
		std::size_t,
		cached_string,
		std::hash<std::size_t>,
		std::equal_to<std::size_t>,
		io::h_allocator< std::pair<const std::size_t, cached_string> >
		> pool_type;
	pool_type pool_;
};

} // namespace io

#endif // __IO_STRINGPOOL_HPP_INCLUDED__
