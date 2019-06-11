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


namespace detail {

struct long_char_buf_t {
	std::size_t sso: 1;
std::size_t size:
	(sizeof(std::size_t)*CHAR_BIT - 1);
	uint8_t* char_buf;
};

static constexpr std::size_t SSO_MAX = sizeof(long_char_buf_t) - 2;

struct short_char_buf_t {
	bool sso: 1;
	// assuming MAX_SIZE string length is not possible,
uint8_t size:
	CHAR_BIT - 1;
	// max val + zero ending char
	char char_buf[ SSO_MAX+1 ];
};

// Packs small strings into char array instead of allocating heap value
// whenever possible
// 16 bytes for most known compilers
// tested with GCC, clang, intel and ms vc++ 17
union sso_variant_t {
	long_char_buf_t long_buf;
	short_char_buf_t short_buf;
};

constexpr bool is_short(const sso_variant_t& v) noexcept
{
	return v.short_buf.sso;
}

constexpr std::size_t short_size(const sso_variant_t& v) noexcept
{
	return v.short_buf.size;
}

constexpr const char* short_str(const sso_variant_t& v) noexcept
{
	return v.short_buf.char_buf;
}

inline const char* long_str(const sso_variant_t& v) noexcept
{
	return reinterpret_cast<char*>( v.long_buf.char_buf + sizeof(std::size_t) );
}


} // namespace detail

///  \brief Immutable zero ending C style string wrapper
class IO_PUBLIC_SYMBOL const_string final {
private:

	static void intrusive_add_ref(detail::sso_variant_t& var) noexcept {
		std::size_t volatile *p = reinterpret_cast<std::size_t volatile*>(var.long_buf.char_buf);
		detail::atomic_traits::inc(p);
	}

	static std::size_t intrusive_release(detail::sso_variant_t& var) noexcept {
		std::size_t volatile *p = reinterpret_cast<std::size_t volatile*>(var.long_buf.char_buf);
		return detail::atomic_traits::dec(p);
	}

	// is short string optimized version
	inline bool sso() const noexcept {
		return detail::is_short(data_);
	}

	static inline bool carr_empty(const char* rhs, std::size_t len) noexcept {
		return 0 == len || nullptr == rhs || '\0' == *rhs;
	}

public:

	typedef std::char_traits<char> traits_type;

	/// Creates empty constant string  object
	constexpr const_string() noexcept:
		data_( {false, 0, nullptr} )
	{}

	const_string(const const_string& other) noexcept:
		data_(other.data_) {
		// increase reference count if needed
		// for long buffer
		if( !empty() && !sso() )
			intrusive_add_ref(data_);
	}

	const_string& operator=(const const_string& rhs) noexcept {
		const_string(rhs).swap( *this );
		return *this;
	}

	/// Movement constructor, default movement semantic
	const_string(const_string&& other) noexcept:
		data_(other.data_) {
		other.data_ = {false,0,nullptr};
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
	const_string(const char* str, std::size_t length) noexcept;

	~const_string() noexcept {
		// check for sso, end empty string
		// decrement atomic intrusive reference counter
		// release long buffer if needed
		if( !empty() && !sso() && 0 == intrusive_release(data_) )
			memory_traits::free(data_.long_buf.char_buf);
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

	/// Swaps this object with another one
	/// \param with object to swap with this
	inline void swap(const_string& with) noexcept {
		std::swap(data_, with.data_);
	}

	/// Returns whether this string is pointing on nullptr
	/// \return whether nullptr string
	inline bool empty() const noexcept {
		return !data_.short_buf.sso && 0 == data_.long_buf.size && nullptr == data_.long_buf.char_buf;
	}

	/// Checks whether this string empty or contains only whitespace characters
	/// \return whether this string is blank
	inline bool blank() const noexcept {
		if( !empty() ) {
			char *c;
			for(c = const_cast<char*>( data() ); io_isspace(*c); ++c);
			return '\0' == *c;
		}
		return true;
	}

	/// Returns raw C-style zero ending string
	/// \return C-style string, "" if string is empty
	const char* data() const noexcept {
		return empty() ? "" : sso() ? detail::short_str(data_) : detail::long_str(data_);
	}

	/// Converts this string to system UCS-2 ( UTF-16 LE or BE)
	inline std::u16string convert_to_u16() const {
		return empty() ? std::u16string() : transcode_to_u16( data(), size() );
	}

	/// Converts this string to system UCS-4 ( UTF-32 LE or BE)
	inline std::u32string convert_to_u32() const {
		return empty() ? std::u32string() : transcode_to_u32( data(), size() );
	}

	/// Converts this string to system wide UNICODE (UTF-16/32 LE/BE OS and CPU byte order depends) representation
	inline std::wstring convert_to_ucs() const {
		return empty() ? std::wstring() : transcode_to_ucs( data(), size() );
	}

	///Returns string length in UNICODE characters
	/// \return string length in characters
	inline std::size_t length() const noexcept {
		return empty() ? 0 : utf8::strlength( data() );
	}


	/// Returns string size in bytes
	/// \return string size in bytes
	inline std::size_t size() const noexcept {
		return empty() ? 0 : sso() ? detail::short_size(data_) : data_.long_buf.size;
	}

	/// Hash this string bytes (murmur3 for 32bit, Cityhash for 64 bit)
	/// \return string content hash
	inline std::size_t hash() const noexcept {
		return empty() ? 0 : io::hash_bytes( data(), size() );
	}

	/// Lexicographically compare the string with another
	/// \param rhs a string to compare with
	/// \return whether strings equals
	bool operator==(const const_string& rhs) const noexcept {
		return 0 == compare( rhs );
	}

	bool operator<(const const_string& rhs) const noexcept {
		return compare( rhs ) < 0;
	}

	bool operator>(const const_string& rhs) const noexcept {
		return compare( rhs ) > 0;
	}

	inline bool equal(const char* rhs) const noexcept {
		return equal( rhs, nullptr != rhs ? traits_type::length(rhs) : 0 );
	}

	bool equal(const char* rhs, const std::size_t len) const noexcept {
		if(carr_empty(rhs, len) && empty() )
			return true;
		else if( !empty() && !carr_empty(rhs, len) && len == size() )
			return 0 == traits_type::compare( data(), rhs, len );
		else
			return false;
	}

private:

	int compare(const const_string& rhs) const noexcept {
		if( !sso() && !rhs.sso() && data_.long_buf.char_buf == rhs.data_.long_buf.char_buf)
			return 0;
		else if( empty() && rhs.empty() )
			return 0;
		else if( size() == rhs.size() )
			return traits_type::compare( data(), rhs.data(), size() );
		else if( size() < rhs.size() )
			return -1;
		return 1;
	}

private:
	detail::sso_variant_t data_;
};

inline std::ostream& operator<<(std::ostream& os, const const_string& cstr)
{
	os.write( cstr.data(), cstr.size() );
	return os;
}

inline std::wostream& operator<<(std::wostream& os, const const_string& cstr)
{
	std::wstring conv = cstr.convert_to_ucs();
	os.write(conv.data(), conv.size() );
	return os;
}

inline std::basic_ostream<char16_t>& operator<<(std::basic_iostream<char16_t>& os, const const_string& cstr)
{
	std::u16string conv = cstr.convert_to_u16();
	os.write( conv.data(), conv.size() );
	return os;
}

inline std::basic_ostream<char32_t>& operator<<(std::basic_iostream<char32_t>& os, const const_string& cstr)
{
	std::u32string conv = cstr.convert_to_u32();
	os.write( conv.data(), conv.size() );
	return os;
}

} // namespace io


#endif // __IO_CONSTSTRING_HPP_INCLUDED__
