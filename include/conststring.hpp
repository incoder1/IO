/*
 * Copyright (c) 2016-2019
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

#include <cstring>
#include <cctype>
#include <functional>
#include <ostream>
#include <string>

#if defined(__HAS_CPP_17)
#	include <string_view>
#endif

namespace io {


namespace detail {

#if defined(__HAS_CPP_17) && defined(__cpp_char8_t)
typedef char8_t utf8char;
#else
typedef uint8_t utf8char;
#endif // defined

struct long_char_buf_t {
	// considering size would never be larger then SSIZE_MAX
	// since most systems have much less memory then CPU can address
	// so use signbit as align for sso boolean
	std::size_t sso: 1;
	std::size_t size: (sizeof(std::size_t)*CHAR_BIT - 1);
	// layout for string is {reference_counter:size_t}{char_data}{'\0'}
	utf8char* char_buf;
};

// two bytes reserver to SSO flag and size
static constexpr std::size_t SSO_MAX = sizeof(long_char_buf_t) - 2;

struct short_char_buf_t {
	bool sso: 1;
	// assuming MAX_SIZE string length is not possible,
    uint8_t size: CHAR_BIT - 1;
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
	return reinterpret_cast<char*>(v.long_buf.char_buf) + sizeof(std::size_t);
}


} // namespace detail

///  \brief Immutable zero ending C style string wrapper
class IO_PUBLIC_SYMBOL const_string final {
private:

	static void long_buf_add_ref(detail::sso_variant_t& var) noexcept {
		std::size_t volatile *p = reinterpret_cast<std::size_t volatile*>(var.long_buf.char_buf);
		// increment string intrusive reference counter, with relaxed memory order
		detail::atomic_traits::inc(p);
	}

	static inline bool carr_empty(const char* rhs, std::size_t len) noexcept {
		return 0 == len || nullptr == rhs || '\0' == *rhs;
	}

	static void long_buf_release(detail::sso_variant_t& var) noexcept;

	void init_short(detail::sso_variant_t& dst, const char* str, std::size_t length) noexcept;
	void init_long(detail::sso_variant_t& dst,std::size_t size,const char* str, std::size_t length) noexcept;

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
			long_buf_add_ref(data_);
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

	~const_string() noexcept
	{
		if( !empty() && !sso() )
			long_buf_release(data_);
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

	/// Check this sting is short string optimized version
	/// \return whether short string optimized
	inline bool sso() const noexcept {
		return detail::is_short(data_);
	}

	/// Returns whether this string is pointing on nullptr
	/// \return whether nullptr string
	inline bool empty() const noexcept {
		return !sso() && 0 == data_.long_buf.size;
	}

	/// Checks whether this string empty or contains only whitespace characters
	/// \return whether this string is blank
	inline bool blank() const noexcept {
		constexpr const char* WS = "\t\n\v\f\r ";
		return empty() ? true : size() == io_strspn(data(), WS);
	}

	/// Returns raw C-style zero ending string
	/// \return C-style string, "" if string is empty
	const char* data() const noexcept {
		return empty() ? "" : sso() ? detail::short_str(data_) : detail::long_str(data_);
	}

	/// Returns mutable std::string by deep copying underlying character array
	inline std::string stdstr() const {
		return std::string( data(), size() );
	}

#ifdef __HAS_CPP_17
	inline std::string_view get_view() const  {
		return std::string_view( data(), size() );
	}
#endif // __HAS_CPP_17


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

	inline bool ptr_equal(const const_string& rhs) const noexcept {
		return (this == std::addressof(rhs) ) || ( !sso() && !rhs.sso() && data_.long_buf.char_buf == rhs.data_.long_buf.char_buf);
	}

	int compare(const const_string& rhs) const noexcept {
		if( ( empty() && rhs.empty() ) || ptr_equal(rhs) )
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

#ifndef _MSC_VER
class const_string_hash: public std::unary_function<std::size_t,io::const_string> {
public:
	inline std::size_t operator()(const io::const_string& str) const noexcept {
		return str.hash();
	}
};
#else
class const_string_hash {
public:
	typedef io::const_string argument_type;
    typedef std::size_t result_type;
	inline std::size_t operator()(const io::const_string& str) const noexcept {
		return str.hash();
	}
};
#endif


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
