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
#ifndef __IO_HASHING_HPP_INCLUDED__
#define __IO_HASHING_HPP_INCLUDED__

#include "config.hpp"

#include <utility>

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

namespace io {

/// Improved hash function for strings
/// Provides Google CityHash for 64 bit or MurMur3A for 32 bit CPU
/// \param array of bytes
/// \param count array size in bytes
/// \return hash value
std::size_t IO_PUBLIC_SYMBOL hash_bytes(const uint8_t* bytes, std::size_t count) noexcept;

template<typename T>
inline std::size_t hash_bytes(const T* bytes,std::size_t count) noexcept {
	return hash_bytes( reinterpret_cast<const uint8_t*>(bytes), (count*sizeof(T)) );
}

template <typename T>
constexpr inline void hash_combine(std::size_t& seed,const T& v) noexcept
{
	static_assert(std::is_arithmetic<T>::value && !std::is_pointer<T>::value, " Only arithmetic non pointers");
	const constexpr std::size_t PRIME =  0x9E3779B9;
	seed ^= v + PRIME + ( seed << 6 ) + ( seed >> 2);
}

} // namespace io

#endif // __IO_HASHING_HPP_INCLUDED__
