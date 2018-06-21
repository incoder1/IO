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

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include "text.hpp"
#include "hashing.hpp"

#include <ostream>
#include <cstring>
#include <string>
#include <cctype>

namespace io {

///  \brief Immutable zero ending C style string wrapper
class const_string final {
private:
	static inline void intrusive_add_ref(uint8_t* const ptr) noexcept {
		std::size_t volatile *p = reinterpret_cast<std::size_t volatile*>(ptr);
		detail::atomic_traits::inc(p);
	}
	static inline bool intrusive_release(uint8_t* const ptr) noexcept {
		std::size_t volatile *p = reinterpret_cast<std::size_t volatile*>(ptr);
		return static_cast<size_t>(0) == detail::atomic_traits::dec(p);
	}

#ifdef IO_IS_LITTLE_ENDIAN
	static constexpr unsigned int MB_SHIFT = ( sizeof(unsigned int) << 3 ) - 8;
#endif // IO_IS_LITTLE_ENDIAN

	// returns UTF-8 character size in bytes
	static unsigned int u8_mblen(const uint8_t* mb) noexcept {
		if( static_cast<unsigned int>(mb[0]) < 0x80U)
			return 1;
#ifdef IO_IS_LITTLE_ENDIAN
		unsigned int c = static_cast<unsigned int>(mb[0]) << MB_SHIFT;
#else
		unsigned int c = static_cast<unsigned int>(mb[0]);
#endif // IO_IS_LITTLE_ENDIAN
		return static_cast<unsigned int>( io_clz( ~c ) );
	}

public:

	/// Returns UTF-8 string length in logical UNICODE characters
	/// \return length in characters
	static std::size_t utf8_length(const char* u8str) noexcept {
		std::size_t ret = 0;
		for(const uint8_t *c = reinterpret_cast<const uint8_t*>(u8str); '\0' != *c; c += u8_mblen(c) )
			++ret;
		return ret;
	}

	typedef std::char_traits<char> traits_type;

	/// Creates empty constant string  object
	constexpr const_string() noexcept:
		data_(nullptr)
	{}

	/// Shallow copy this string (inc reference count)
	const_string(const const_string& other):
		data_(other.data_) {
		if(nullptr != data_)
			// increase reference count
			intrusive_add_ref(data_);
	}

	/// Copy assignment operator, shallow copy this string
	const_string& operator=(const const_string& rhs) {
		const_string(rhs).swap( *this );
		return *this;
	}

	/// Movement constructor, default movement semantic
	const_string(const_string&& other) noexcept:
		data_(other.data_) {
		other.data_ = nullptr;
	}

	/// Movement assignment operator, default movement semantic
	const_string& operator=(const_string&& other) noexcept {
		const_string( std::forward<const_string>(other) ).swap( *this );
		return *this;
	}

	/// Constructs constant string object by deep copying a character array
	/// \param str pointer to character array begin
	/// \param length count of bytes to copy from array
	/// \throw never throws, constructs empty string if no free memory left
	const_string(const char* str, std::size_t length) noexcept:
		data_(nullptr) {
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

	/// Decrement this string reference count, release allocated memory when
	/// reference count bring to 0
	~const_string() noexcept {
		if(nullptr != data_ && intrusive_release(data_) )
			memory_traits::free(data_);
	}

	/// Deep copy a continues memory block (character array)
	/// \param first pointer on memory block begin
	/// \param last pointer on block end
	const_string(const char* first, const char* last) noexcept:
		const_string( first, memory_traits::distance(first,last) )
	{}

	/// Deep copies a zero ending C string
	const_string(const char* str) noexcept:
		const_string(str, traits_type::length(str) )
	{}

	/// Deep copy STD lib string
	/// \param str STD lib string to be copied
	const_string(const std::string& str) noexcept:
		const_string( str.data(), str.length() )
	{}

	/// Swaps this object with another one
	/// \param with object to swap with this
	inline void swap(const_string& with) noexcept {
		std::swap(data_, with.data_);
	}

	/// Returns whether this string is pointing on nullptr
	/// \return whether nullptr string
	inline bool empty() const noexcept {
		return nullptr == data_;
	}

	/// Checks whether this string empty or contains only whitespace characters
	/// \return whether this string is blank
	inline bool blank() const noexcept {
		if( !empty() ) {
			const char *c = data();
			while( !io_isspace( traits_type::to_int_type( *c ) ) )
				++c;
			return '\0' != *c;
		}
		return true;
	}

	/// Returns raw C-style zero ending string
	/// \return C-style string, "" if string is empty
	inline const char* data() const noexcept {
		return nullptr == data_ ? "" : reinterpret_cast<char*>( data_ + sizeof(std::size_t) );
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
		return empty() ? std::u16string() : transcode_to_u16( data(), size() );
	}

	/// Converts this string to system UCS-4 ( UTF-32 LE or BE)
	inline std::u32string convert_to_u32() const {
		return empty() ? std::u32string() : transcode_to_u32( data(), size() );
	}

	/// Converts this string to system whide UNICODE (UTF-16/32 LE/BE OS and CPU byte order depends) representation
	inline std::wstring convert_to_ucs() const {
		return empty() ? std::wstring() : transcode_to_ucs( data(), size() );
	}

	///Returns string length in UNICODE characters
	/// \return string length in characters
	inline std::size_t length() const noexcept {
		return empty() ? 0 : utf8_length( data() );
	}


	/// Returns string size in bytes
	/// \return string size in bytes
	inline std::size_t size() const noexcept {
		return empty() ? 0 : traits_type::length( data() );
	}

	/// Hash this string bytes (murmur3 for 32bit, cityhash for 64 bit)
	/// \return string content hash
	inline std::size_t hash() const noexcept {
		return empty() ? io::hash_bytes( data(), size() ) : 0;
	}

	/// Lexicographically compare the string with another
	/// \param rhs a string to compare with
	/// \return whether strings equals
	bool operator==(const const_string& rhs) const noexcept {
		return 0 == compare( rhs );
	}

	bool operator<(const const_string& rhs) const noexcept {
		return 0 > compare( rhs );
	}

	bool operator>(const const_string& rhs) const noexcept {
		return 0 < compare( rhs );
	}

private:
	inline int compare(const const_string& rhs) const noexcept {
		if(data_ == rhs.data_)
			return 0;
		else if( empty() && !rhs.empty() )
			return -1;
		else if( !empty() && rhs.empty() )
			return 1;
		else
			return std::strcmp( data(), rhs.data() );
	}
private:
	uint8_t* data_;
};

inline std::ostream& operator<<(std::ostream& os, const const_string& cstr)
{
	return ( os << cstr.data() );
}

inline std::wostream& operator<<(std::wostream& os, const const_string& cstr)
{
	return ( os << cstr.convert_to_ucs() );
}

inline std::basic_ostream<char16_t>& operator<<(std::basic_iostream<char16_t>& os, const const_string& cstr)
{
	return ( os << cstr.convert_to_u16() );
}

inline std::basic_ostream<char32_t>& operator<<(std::basic_iostream<char32_t>& os, const const_string& cstr)
{
	return ( os << cstr.convert_to_u32() );
}

} // namespace io


#endif // __IO_CONSTSTRING_HPP_INCLUDED__
