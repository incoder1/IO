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
#include "text.hpp"

#include <atomic>
#include <ostream>
#include <string>
#include <unordered_map>

#include "hashing.hpp"
#include "object.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

namespace io {

class cached_string;

namespace detail {

	class char_holder
	{
		char_holder(const char_holder&) = delete;
		char_holder& operator=(const char_holder&) = delete;
		void* operator new(std::size_t) = delete;
	public:

		__forceinline const char* data() const noexcept {
			return data_;
		}

		static inline void* operator new(std::size_t, void* p) noexcept
		{
			assert(nullptr != p);
			return p;
		}

		static inline void operator delete(void* const p, std::size_t) noexcept
		{
			assert(nullptr != p);
			memory_traits::free(p);
		}

		~char_holder() noexcept = default;
	public:
		constexpr char_holder(const char* data) noexcept:
			ref_count_(1),
			data_(data)
		{}
		static char_holder* alloc(const char* s,const std::size_t len) noexcept;
		static inline void add_ref(char_holder* const hld) noexcept {
           hld->ref_count_.fetch_add(1, std::memory_order_relaxed);
		}
		static inline void release(char_holder* const hld) noexcept {
			if(1 == hld->ref_count_.fetch_sub(1, std::memory_order_acquire) ) {
				std::atomic_thread_fence( std::memory_order_release);
				delete hld;
			}
		}
	private:
		std::atomic_size_t ref_count_;
		const char *data_;
	};

} // namespace detail

class string_pool;

 /// \brief A holder of a zerro terminated character array stored in string pool.
 /*!
 * \details Two or more holders can point on the same character array at time.
 * Character array will be stored in memory since at list
 * one caches_string object in stack (or packed into a heap class/struct) and string_pool object is not destroyed.
 */
class IO_PUBLIC_SYMBOL cached_string
{
private:
	typedef std::char_traits<char> traits_type;
public:

	/// Careates an "" empty chached string  object
	constexpr cached_string() noexcept:
		holder_(nullptr)
	{}

	/// Copy constructor, just increases string reference count
	cached_string(const cached_string& rhs) noexcept:
			holder_(rhs.holder_)
	{
		if( nullptr != holder_ )
			detail::char_holder::add_ref(holder_);
	}

	/// Copy assignment operator, just increases string reference count
	cached_string& operator=(const cached_string& rhs) noexcept
	{
		cached_string(rhs).swap( *this );
		return *this;
	}

	/// Decrements string reference count, releases string memory when reference count becomes 1
	inline ~cached_string() noexcept
	{
		if(nullptr != holder_)
			detail::char_holder::release(holder_);
	}

	cached_string(cached_string&& rhs) noexcept:
		holder_(rhs.holder_)
	{
		rhs.holder_ = nullptr;
	}

	inline cached_string& operator=(cached_string&& rhs) noexcept
	{
		cached_string( std::forward<cached_string>(rhs) ).swap( *this );
		return *this;
	}

	/// Swaps two chached_string objects
	/// \param with cached_string object to swap with this
	inline void swap(cached_string& with) noexcept {
		std::swap(holder_, with.holder_);
	}

	/// Returns whether this string is pointing on nullptr
	/// \retrun whether nullptr string
	inline bool empty() const noexcept {
		return nullptr == holder_;
	}

	/// Returns raw C-style zero ending string
	/// \return C-style string, "" if string is empty
	inline const char* data() const noexcept {
		return nullptr == holder_ ? "" : holder_->data();
	}

#ifndef NDEBUG
	/// Returns raw C-style zero ending string same as data(), provided for IDE's and debuggers
	/// \return C-style string "" if string is empty
	inline const char* c_str() const noexcept {
		return data();
	}
#endif // NDEBUG

	/// Returns string length in bytes
	/// \return string length in bytes
	inline std::size_t length() const noexcept {
		return traits_type::length( data() );
	}

	/// Hash this string bytes (murmur3 for 32bit, cityhash for 64 bit)
	/// \return string content hash
	inline std::size_t hash() const noexcept {
		return holder_ != 0 ?  io::hash_bytes( data(), length() ) : 0;
	}

	/// Returns pointer on this string, as hash values.
	/// Since string is unique this can be faster for single hash table,
	/// but result will differ from time to time
	/// \return string pointer value
	inline std::size_t fast_hash() const noexcept {
		return reinterpret_cast<std::size_t>(holder_);
	}

	/// Compares two cached_string objects
	/// \param rhs cached_string object to compare
	/// \return true if objects equals, false otherwise
	inline bool operator==(const cached_string& rhs) const noexcept {
		return holder_ == rhs.holder_;
	}

	/// Compares two cached_string objects
	/// \param rhs cached_string object to compare
	/// \return true if objects not equals, false otherwise
	inline bool operator!=(const cached_string& rhs) const noexcept {
		return holder_ != rhs.holder_;
	}

	/// Checks this string is binary equals with in character array
	/// \param s character array to check
	/// \param bytes character array size in bytes
	/// \return whether this string is equals with character array
	inline bool eq(const char* s, std::size_t bytes) const noexcept {
		return 0 == traits_type::compare( s, data(), bytes);
	}

	/// Checks this string is binary equals with zero terminated C string
	/// \param s zero terminated C string
	/// \return whether this string is equals with C string
	inline bool eq(const char* s) const noexcept {
		return 0 == io_strcmp(data(), s);
	}

	/// Converts this string to system UCS-2 ( UTF-16 LE or BE)
	inline std::u16string convert_to_u16() const {
		return empty() ? std::u16string() : transcode_to_u16( data(), length() );
	}

	/// Converts this string to system UCS-4 ( UTF-32 LE or BE)
	inline std::u32string convert_to_u32() const {
		return empty() ? std::u32string() : transcode_to_u32( data(), length() );
	}

	/// Converts this string to system whide UNICODE (UTF-16/32 LE/BE OS and CPU byte order depends) representation
	inline std::wstring convert_to_ucs() const {
		return empty() ? std::wstring() : transcode_to_ucs( data(), length() );
	}

private:
	friend class string_pool;
	cached_string(const char* s, std::size_t count) noexcept;
private:
	detail::char_holder* holder_;
};

/// ostream output operator
inline std::ostream& operator<<(std::ostream& os,const cached_string& str) {
	return os << str.data();
}

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
	string_pool();
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
