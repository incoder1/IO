/*
 *
 * Copyright (c) 2016-2019
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef __INTRUSIVE_ARRAY_HPP_INCLUDED__
#define __INTRUSIVE_ARRAY_HPP_INCLUDED__

#include <config.hpp>
#include <type_traits>

namespace util {

template<typename T, class = typename
		std::enable_if<
				std::is_copy_assignable<T>::value || std::is_move_assignable<T>::value
			>::type
	>
class intrusive_array {
private:
	static inline void intrusive_add_ref(uint8_t* const ptr) noexcept {
		std::size_t volatile *p = reinterpret_cast<std::size_t volatile*>(ptr);
		io::detail::atomic_traits::inc(p);
	}
	static inline bool intrusive_release(uint8_t* const ptr) noexcept {
		std::size_t volatile *p = reinterpret_cast<std::size_t volatile*>(ptr);
		return static_cast<size_t>(0) == io::detail::atomic_traits::dec(p);
	}
public:

	constexpr intrusive_array() noexcept:
		length_(0),
		mem_(nullptr)
	{}

	explicit intrusive_array(std::size_t len) noexcept:
		length_(len),
		mem_(nullptr)
	{
		mem_ = io::memory_traits::malloc_array<uint8_t>(sizeof(std::size_t) + ( length_ * sizeof(T) ) );
		if(nullptr != mem_)
			intrusive_add_ref(mem_);
	}

	intrusive_array(const T* src, std::size_t len) noexcept:
		length_(len),
		mem_(nullptr)
	{
		const std::size_t bytes = length_ * sizeof(T);
		mem_ = static_cast<uint8_t*>(io::memory_traits::malloc(sizeof(std::size_t) + bytes));
		if(nullptr != mem_) {
			intrusive_add_ref(mem_);
			io_memmove( (mem_ + sizeof(std::size_t)), src, bytes );
		}
	}


	intrusive_array(const intrusive_array& other) noexcept:
		length_(other.length_),
		mem_(other.mem_)
	{
		if(nullptr != mem_)
			// increase reference count
			intrusive_add_ref(mem_);
	}

	intrusive_array& operator=(const intrusive_array& rhs) noexcept {
		intrusive_array(rhs).swap( *this );
		return *this;
	}

	intrusive_array(intrusive_array&& other) noexcept:
		length_(other.length_),
		mem_(other.mem_)
	{
		other.length_ = 0;
		other.mem_ = nullptr;
	}

	intrusive_array& operator=(intrusive_array&& other) noexcept {
		intrusive_array( std::forward<intrusive_array>(other) ).swap( *this );
		return *this;
	}

	~intrusive_array() noexcept {
		if(nullptr != mem_ && intrusive_release(mem_) ) {
			std::atomic_thread_fence( std::memory_order_acquire  );
			io::memory_traits::free(mem_);
		}
	}

	explicit operator bool() const noexcept {
		return nullptr != mem_;
	}

	inline const T* get() const {
		return reinterpret_cast<T*>( mem_ + sizeof(std::size_t) );
	}

	inline T& operator[](std::size_t idx) noexcept {
		T *px = reinterpret_cast<T*>( mem_ + sizeof(std::size_t) );
        return px[ idx ];
	}

	inline std::size_t length() const noexcept {
		return length_;
	}

	inline std::size_t size() const noexcept {
		return length_ * sizeof(T);
	}

	inline void swap(intrusive_array& with) noexcept {
		std::swap(length_, with.length_);
		std::swap(mem_, with.mem_);
	}

private:
	std::size_t length_;
	uint8_t *mem_;
};

} // namespace util

#endif // __INTRUSIVE_ARRAY_HPP_INCLUDED__
