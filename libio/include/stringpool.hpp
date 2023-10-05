/*
 *
 * Copyright (c) 2016-2020
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

	typedef h_allocator< std::pair<const std::size_t, const_string> > allocator_type;

	typedef std::unordered_map<
		std::size_t,
		const_string,
		std::hash<std::size_t>,
		std::equal_to<std::size_t>,
		allocator_type
		> pool_type;

	/// Construct new string pool
	explicit string_pool(pool_type&& pool) noexcept;

	const_string load(const std::size_t str_hash, const char* s,const std::size_t size) noexcept;

	bool find(const_string& str,const std::size_t str_hash, const char* s,const std::size_t size) noexcept;

	const_string find_or_load(const char* s,const std::size_t size) noexcept;

public:

	static s_string_pool create(std::error_code& ec) noexcept;

	virtual ~string_pool() noexcept override;

	/// Returns a cached constant string object for the raw character array.
	/// \param s source character array
	/// \param size size of array in bytes
	/// \return a constant string with the cached value or empty string when no free RAM left
	/// \throw never throws
	const  const_string get(const char* s,const std::size_t size) noexcept;

	/// Returns a cached constant string object for the C zero ending string
	/// \param s source zero terminated C string
	/// \return a constant string with the cached value or empty string when no free RAM left, or s points to "" or nullptr
	/// \throw never throws
	inline const const_string get(const char* s) noexcept
	{
		return get(s, const_string::traits_type::length(s) );
	}

	/// Returns count of strings cached by this pool
	/// \return count of strings
	inline std::size_t size() const noexcept
	{
		return pool_.size();
	}

private:

	pool_type pool_;

};

} // namespace io

#endif // __IO_STRINGPOOL_HPP_INCLUDED__
