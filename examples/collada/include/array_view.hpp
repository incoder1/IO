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
#ifndef __ARRAY_VIEW_HPP_INCLUDED__
#define __ARRAY_VIEW_HPP_INCLUDED__

namespace util {

/// A non-owning reference to an raw array
template<class T>
class array_view
{
	public:
		constexpr array_view(const T* array, const std::size_t size) noexcept:
			px_(array),
			size_(size)
		{}
		constexpr array_view() noexcept:
			array_view(nullptr, 0)
		{}
		explicit operator bool() const noexcept {
			return nullptr != px_;
		}
		const T* get() const noexcept {
			return px_;
		}
		const std::size_t size() const noexcept {
			return size_;
		}
		const std::size_t bytes() const noexcept {
			return sizeof(T) * size_;
		}
	protected:
		const T* px_;
		const std::size_t size_;
};

} // namespace util

#endif // __ARRAY_VIEW_HPP_INCLUDED__
