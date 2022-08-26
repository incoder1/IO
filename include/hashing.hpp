/*
 *
 * Copyright (c) 2016-2020
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

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include "type_traits_ext.hpp"

namespace io {

/// Improved hash function for strings
/// Provides Google CityHash for 64 bit or MurMur3A for 32 bit CPU
/// \param array of bytes
/// \param count array size in bytes
/// \return hash value
std::size_t IO_PUBLIC_SYMBOL hash_bytes(const uint8_t* bytes, std::size_t count) noexcept;


#ifdef IO_HAS_CONNCEPTS
template<typename _Tp>
concept fundamental_hashable_type = std::is_integral_v<_Tp> || std::is_floating_point_v<_Tp>;

template<typename T>
	requires( fundamental_hashable_type<T> )
#else
template<typename T,
	typename std::enable_if<
		std::is_integral<T>::value || std::is_floating_point<T>::value
		>::type* = nullptr
	>
#endif // IO_HAS_CONNCEPTS
inline std::size_t hash_bytes(const T* bytes,std::size_t count) noexcept {
	return hash_bytes( reinterpret_cast<const uint8_t*>(bytes), (count*sizeof(T)) );
}

/// Combine two hashes
/// \param seed basic hashing value
/// \param next a hash to combine with seed
void IO_PUBLIC_SYMBOL hash_combine(std::size_t& seed,const std::size_t next) noexcept;

} // namespace io

#endif // __IO_HASHING_HPP_INCLUDED__
