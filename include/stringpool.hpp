/*
 * Copyright (c) 2016-2019
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

#include <algorithm>
#include <ostream>
#include <string>
#include <unordered_map>

#include "conststring.hpp"
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
class IO_PUBLIC_SYMBOL string_pool final:public object {
	string_pool(const string_pool&) = delete;
	string_pool& operator=(const string_pool&) = delete;
private:

	/// Construct new string pool
	explicit string_pool() noexcept;
public:

	static s_string_pool create(std::error_code& ec) noexcept;

	virtual ~string_pool() noexcept override;

	/// Returns a cached_string object for the raw character array.
	/// \param s source character array
	/// \param size size of array in bytes
	/// \return cached_string object or empty cached string if out of memory or size is 0
	/// \throw never throws
	const cached_string get(const char* s, std::size_t count) noexcept;

	/// Returns a cached_string object for the C zero ending string
	/// \param s source zero terminated C string
	/// \return cached_string object or empty cached string if out of memory or s is "" or nullptr
	/// \throw never throws
	inline const cached_string get(const char* s) noexcept {
		return get(s, cached_string::traits_type::length(s) );
	}

	/// Returns count of strings cached by this pool
	/// \return count of strings
	inline std::size_t size() const noexcept {
		return pool_.size();
	}

private:
	friend class nobadalloc<string_pool>;
#ifdef __IO_WINDOWS_BACKEND__
	typedef enclave_allocator< std::pair<const std::size_t, cached_string> > allocator_type;
#else
	typedef h_allocator< std::pair<const std::size_t, cached_string> > allocator_type;
#endif // __IO_WINDOWS_BACKEND__

	typedef std::unordered_map<
		std::size_t,
		cached_string,
		std::hash<std::size_t>,
		std::equal_to<std::size_t>,
		allocator_type
		> pool_type;

	pool_type pool_;

};

} // namespace io

#endif // __IO_STRINGPOOL_HPP_INCLUDED__
