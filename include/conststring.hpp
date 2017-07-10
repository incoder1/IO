/*
 * Copyright (c) 2016
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
*/
#ifndef __IO_CONSTSTRING_HPP_INCLUDED__
#define __IO_CONSTSTRING_HPP_INCLUDED__

#include "config.hpp"
#include "text.hpp"
#include "hashing.hpp"

#include <ostream>
#include <string>

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

namespace io {

///  \brief Immutable heap stored zero ending string wrapper
class const_string {
public:
	typedef std::char_traits<char> traits_type;

	/// Careates an "" empty const string  object
	constexpr const_string() noexcept:
		data_(nullptr)
	{}

	/// Deep copy a continues memory block (character array)
	/// \param first pointer on memory block begin
	/// \param last pointer on block end
	const_string(const char* first, const char* last) noexcept:
		data_(nullptr)
	{
		if(first < last) {
			std::size_t len = detail::distance(first, last) + 1;
			data_ = static_cast<char*>( io::h_malloc(len) );
			if( nullptr != data_) {
				traits_type::copy(data_, first, len-1);
				data_[len] = '\0';
			}
		}
	}

	/// Deep copy a character array
	const_string(const char* str, std::size_t length) noexcept:
		const_string(str, str+length)
	{}

	/// Deep copies a zero ending C string
	const_string(const char* str) noexcept:
		const_string(str, io_strlen(str) )
	{}

	/// Copy constructor, deep copies string content
	const_string(const const_string& rhs):
		data_(nullptr)
	{
		if( ! rhs.empty() ) {
			std::size_t len = rhs.length() + 1;
			data_ = static_cast<char*>( io::h_malloc(len) );
			if(nullptr != data_) {
				traits_type::copy(data_, rhs.data_, len);
			}
		}
	}

	/// Copy assignment operator, deep copies string content
	const_string& operator=(const const_string& rhs)
	{
		const_string(rhs).swap( *this );
		return *this;
	}

	/// Move assigment constructor, swaps strirng pointers
	const_string(const_string&& rhs) noexcept:
		data_(rhs.data_)
	{
		rhs.data_ = nullptr;
	}

	/// Move assigment operator, swaps strirng pointers
	inline const_string& operator=(const_string&& rhs) noexcept
	{
		const_string( std::forward<const_string>(rhs) ).swap( *this );
		return *this;
	}

	/// Releases string memory if allocated before
	~const_string() noexcept
	{
		if(nullptr != data_)
			io::h_free( data_ );
	}

	/// Swaps two const_string objects
	/// \param with cached_string object to swap with this
	inline void swap(const_string& with) noexcept {
		std::swap(data_, with.data_);
	}

	/// Returns whether this string is pointing on nullptr
	/// \retrun whether nullptr string
	inline bool empty() const noexcept {
		return nullptr == data_;
	}

	/// Returns raw C-style zero ending string
	/// \return C-style string, "" if string is empty
	inline const char* data() const noexcept {
		return nullptr == data_ ? "" : data_;
	}

#ifndef NDEBUG
	/// Returns raw C-style zero ending string same as data(), provided for IDE's and debuggers
	/// \return C-style string "" if string is empty
	inline const char* c_str() const noexcept {
		return data();
	}
#endif // NDEBUG

	/// Converts this string to system UCS-2 ( UTF-16 LE or BE)
	inline std::u16string convert_to_u16() const {
		return empty() ? std::u16string() : transcode_to_u16( data_, length() );
	}

	/// Converts this string to system UCS-4 ( UTF-32 LE or BE)
	inline std::u32string convert_to_u32() const {
		return empty() ? std::u32string() : transcode_to_u32( data_, length() );
	}

	/// Converts this string to system whide UNICODE (UTF-16/32 LE/BE OS and CPU byte order depends) representation
	inline std::wstring convert_to_ucs() const {
		return empty() ? std::wstring() : transcode_to_ucs( data_, length() );
	}

	/// Returns string length in bytes
	/// \return string length in bytes
	inline std::size_t length() const noexcept {
		return empty() ? 0 : traits_type::length( data_ );
	}

	/// Hash this string bytes (murmur3 for 32bit, cityhash for 64 bit)
	/// \return string content hash
	inline std::size_t hash() const noexcept {
		return empty() ?  io::hash_bytes( data_, length() ) : 0;
	}
private:
	char* data_;
};

inline std::ostream& operator<<(std::ostream& os, const const_string& cstr)
{
	os.write( cstr.data(), cstr.length() );
	return os;
}

inline std::wostream& operator<<(std::wostream& os, const const_string& cstr)
{
	return os << cstr.convert_to_ucs();
}

inline std::basic_ostream<char16_t>& operator<<(std::basic_iostream<char16_t>& os, const const_string& cstr)
{
	return os << cstr.convert_to_u16();
}

inline std::basic_ostream<char32_t>& operator<<(std::basic_iostream<char32_t>& os, const const_string& cstr)
{
	return os << cstr.convert_to_u32();
}

} // namespace io

namespace std {

	template<>
	struct hash<io::const_string>
	{
	public:
		std::size_t operator()(const io::const_string& str) {
			return str.hash();
		}
	};

} // namespace std

#endif // __IO_CONSTSTRING_HPP_INCLUDED__
