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

namespace io {

namespace detail {

#ifdef __GNUC__
class atomic_traits {
public:
	static inline std::size_t inc(std::size_t volatile *ptr)
	{
		return __atomic_add_fetch(ptr, 1, __ATOMIC_RELAXED);
	}
	static inline std::size_t dec(std::size_t volatile *ptr)
	{
		return __atomic_sub_fetch(ptr, 1, __ATOMIC_SEQ_CST);
	}
};
#endif // __GNUC__

#ifdef _MSC_VER
class atomic_traits {
public:
#	ifdef _M_X64 // 64 bit instruction set
	static inline std::size_t inc(std::size_t volatile *ptr)
	{
		__int64 volatile *p = reinterpret_cast<__int64 volatile*>(ptr);
#	ifdef _M_ARM
		return static_cast<std::size_t>( _InterlockedIncrement64_nf(p) );
#	else
		return static_cast<std::size_t>( _InterlockedIncrement64(p) );
#	endif // _M_ARM
	}
	static inline std::size_t dec(std::size_t volatile *ptr)
	{
		__int64 volatile *p = reinterpret_cast<__int64 volatile*>(ptr);
		return return static_cast<std::size_t>( _InterlockedDecrement64(p) );
	}
#	else // 32 bit instruction set
	static inline std::size_t inc(std::size_t volatile *ptr)
	{
		_long volatile *p = reinterpret_cast<long volatile*>(ptr);
#	ifdef _M_ARM
		return static_cast<std::size_t>( _InterlockedIncrement_nf(p) );
#	else
		return static_cast<std::size_t>( _InterlockedIncrement(p) );
#	endif // _M_ARM
	}
	static inline std::size_t dec(std::size_t volatile *ptr)
	{
		long volatile *p = reinterpret_cast<long volatile*>(ptr);
		return static_cast<std::size_t>( _InterlockedDecrement(p) );
	}
#	endif // 32 bit instruction set

};
#endif // _MSC_VER

} // namesapace detail

///  \brief Immutable zero ending C style string wrapper
class const_string {
private:
	typedef h_allocator<uint8_t,memory_traits> allocator_type;
	static inline void intrusive_add_ref(uint8_t* ptr) noexcept
	{
		std::size_t volatile *p = reinterpret_cast<std::size_t volatile*>(ptr);
		detail::atomic_traits::inc(p);
	}
	static inline bool intrusive_release(uint8_t* ptr) noexcept
	{
		std::size_t volatile *p = reinterpret_cast<std::size_t volatile*>(ptr);
		return static_cast<size_t>(0) == detail::atomic_traits::dec(p);
	}
public:
	typedef std::char_traits<char> traits_type;

	/// Creates empty const string  object
	constexpr const_string() noexcept:
		data_(nullptr)
	{}

	/// Shallow copy this string (inc reference count)
	const_string(const const_string& other):
		data_(other.data_)
	{
		if(nullptr != data_)
			// increase refference count
			intrusive_add_ref(data_);
	}

	/// Copy assignment operator, shallow copy this string
	const_string& operator=(const const_string& rhs)
	{
		const_string(rhs).swap( *this );
		return *this;
	}

	/// Movement constructor, default movement semantic
	const_string(const_string&& other) noexcept:
		data_(other.data_)
	{
		other.data_ = nullptr;
	}

	/// Movement assigment operator, defailt movement semantic
	const_string& operator=(const_string&& other) noexcept
	{
		const_string( std::forward<const_string>(other) ).swap( *this );
		return *this;
	}

	/// Deep copy a character array
	/// \param str pointer to character array begin
	/// \param length count of chars to be copied
	const_string(const char* str, std::size_t length) noexcept:
		data_(nullptr)
	{
		assert(nullptr != str && length > 0);
		const std::size_t len = sizeof(std::size_t) + length + 1;
		allocator_type all;
		data_ = all.allocate(len);
		io_memset(data_,0,len);
		// set initial intrusive atomic reference count
		intrusive_add_ref( data_ );
		uint8_t *b = data_ + sizeof(std::size_t);
		io_memmove(b, str, length);
	}

	/// Decrement this string refference count, release allocated memory when
	/// reference count bring to 0
	~const_string() noexcept
	{
		if(nullptr != data_ && intrusive_release(data_) ) {
			allocator_type all;
			all.deallocate(data_, 1);
		}
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

	/// Swaps two const_string objects
	/// \param with object to swap with this
	inline void swap(const_string& with) noexcept
	{
		std::swap(data_, with.data_);
	}

	/// Returns whether this string is pointing on nullptr
	/// \retrun whether nullptr string
	inline bool empty() const noexcept
	{
		return nullptr == data_;
	}

	/// Returns raw C-style zero ending string
	/// \return C-style string, "" if string is empty
	inline const char* data() const noexcept
	{
		return nullptr == data_ ? "" : reinterpret_cast<char*>( data_ + sizeof(std::size_t) );
	}

	/// Returns raw C-style zero ending string same as data(), provided for IDE's and debuggers
	/// \return C-style string "" if string is empty
	inline const char* c_str() const noexcept
	{
		return data();
	}

	/// Converts this string to system UCS-2 ( UTF-16 LE or BE)
	inline std::u16string convert_to_u16() const
	{
		return empty() ? std::u16string() : transcode_to_u16( data(), length() );
	}

	/// Converts this string to system UCS-4 ( UTF-32 LE or BE)
	inline std::u32string convert_to_u32() const
	{
		return empty() ? std::u32string() : transcode_to_u32( data(), length() );
	}

	/// Converts this string to system whide UNICODE (UTF-16/32 LE/BE OS and CPU byte order depends) representation
	inline std::wstring convert_to_ucs() const
	{
		return empty() ? std::wstring() : transcode_to_ucs( data(), length() );
	}

	/// Returns string length in bytes
	/// \return string length in bytes
	inline std::size_t length() const noexcept
	{
		return empty() ? 0 : traits_type::length( data() );
	}

	/// Hash this string bytes (murmur3 for 32bit, cityhash for 64 bit)
	/// \return string content hash
	inline std::size_t hash() const noexcept
	{
		return empty() ? io::hash_bytes( data(), length() ) : 0;
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

namespace std {

template<>
struct hash<io::const_string> {
public:
	std::size_t operator()(const io::const_string& str)
	{
		return str.hash();
	}
};

} // namespace std

#endif // __IO_CONSTSTRING_HPP_INCLUDED__
