/*
 *
 * Copyright (c) 2016-2023
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef __IO_TEXT_API_DETAIL_PKG_INT_HPP_INCLUDED__
#define __IO_TEXT_API_DETAIL_PKG_INT_HPP_INCLUDED__

#include <io/config/libio_config.hpp>

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <io/core/type_traits_ext.hpp>

namespace io {

namespace detail {

enum class index_shift {
	bits_4  = 3,
	bits_8  = 2,
	bits_16 = 1
};

enum class shift_mask {
	bits_4  = 7,
	bits_8  = 3,
	bits_16 = 1
};

enum class bit_shift {
	bits_4  = 2,
	bits_8  = 3,
	bits_16 = 4
};

enum class unit_mask {
	bits_4  = 0x0000000FL,
	bits_8  = 0x000000FFL,
	bits_16 = 0x0000FFFFL
};

struct pkg_int {
	index_shift  idxsft;
	shift_mask  sftmsk;
	bit_shift  bitsft;
	unit_mask unitmsk;
	const uint32_t* const data;
	inline uint32_t get(uint32_t idx) const noexcept
	{
		std::size_t i = idx >> static_cast<std::size_t>( idxsft );
		uint32_t rshif = (idx & static_cast<uint32_t>(sftmsk)) << static_cast<uint32_t>(bitsft);
		uint32_t mask = static_cast<std::size_t>(unitmsk);
		return (data[i] >> rshif) & mask;
	}
};

constexpr uint32_t pck_16bits(uint32_t a, uint32_t b) noexcept
{
	return (b << 16) | a;
}

constexpr uint32_t pck_8bits(uint32_t a, uint32_t b, uint32_t c, uint32_t d) noexcept
{
	return pck_16bits( ((b << 8) | a), ((d << 8) | c) );
}

constexpr uint32_t pck_4bits(uint32_t a,uint32_t b,uint32_t c,uint32_t d,uint32_t e,uint32_t f,uint32_t g,uint32_t h) noexcept
{
	return pck_8bits( ((b << 4) | a), ((d << 4) | c), ((f << 4) | e), ((h << 4) | g) );
}

} // namespace detail

} // namespace io

#endif // __IO_TEXT_API_DETAIL_PKG_INT_HPP_INCLUDED__
