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
#include "stdafx.hpp"
#include "hashing.hpp"

namespace io {

// USE MurMur3A for 32 bit instruction set
#ifndef IO_CPU_BITS_64

// murmur3 hashing
namespace murmur3 {

// an randomly selected number seed
static constexpr uint32_t SEED = 0xCAFEBABE;

static constexpr uint32_t C1 = 0xCC9E2D51;
static constexpr uint32_t C2 = 0x1B873593;
static constexpr uint32_t R1 = 15;
static constexpr uint32_t R2 = 13;
static constexpr uint32_t M = 5;
static constexpr uint32_t N = 0xE6546B64;

static constexpr uint32_t MIX_MAGIC_0 = 0x85EBCA6B;
static constexpr uint32_t MIX_MAGIC_1 = 0xC2B2AE35;

static __forceinline uint32_t rotl(uint32_t val, uint8_t shift)
{
	return shift == 0 ? val : (val << shift) | ( val >> (32 - shift) ) ;
}

static inline uint32_t mur(uint32_t k, uint32_t h)
{
	k *= C1;
	k = rotl(k, R1);
	k *= C2;
	h ^= k;
	return rotl(h, R2) * M + N;
}

static inline uint32_t mur_tail(uint32_t t, uint32_t h)
{
	t *= C1;
	t = rotl(t,R1);
	t *= C2;
	return h ^ t;
}

static inline uint32_t tail_dword(const uint8_t *tail, uint32_t mod)
{
	uint32_t result = 0;
	// fill li endian integer
	switch(mod) {
	case 3:
		result ^= *(tail+3) << 16;
	case 2:
		result ^= *(tail+2) << 8;
	}
	result ^= *tail;
	return result;
}

static inline uint32_t final_mix(uint32_t h)
{
	h ^= (h >> 16);
	h *= MIX_MAGIC_0;
	h ^= (h >> 13);
	h *= MIX_MAGIC_1;
	h ^= (h >> 16);
	return h;
}

static uint32_t hash(const uint8_t* key, std::size_t size) noexcept
{
	uint32_t result = SEED;
	const uint32_t *bstrt = reinterpret_cast<const uint32_t*>(key);
	const uint32_t *bend = bstrt + (size >> 2);
	// hash dword blocks
	const uint32_t *i = bstrt;
	while ( i <= bend ) {
		result = mur(*i, result);
		++i;
	}
	// hash tail bytes
	uint32_t mod = size & 3;
	if(mod != 0) {
		const uint8_t *tail = reinterpret_cast<const uint8_t*>(  (bend > bstrt) ? bend+1 : bend);
		result =  mur_tail( tail_dword(tail, mod), result);
	}
	// finalize
	result ^= size;
	return final_mix(result);
}

}

// USE Google CityHash for 64 bit instruction set
#else // 64 bit instruction set

// Original hash function can be found https://github.com/google/cityhash
namespace cityhash {

static constexpr uint64_t K0 = 0xC3A5C85C97CB3127ULL;
static constexpr uint64_t K1 = 0xB492B66FBE98F273ULL;
static constexpr uint64_t K2 = 0x9AE16A3B2F90404FULL;
static constexpr uint64_t K_MUL = 0x9DDFEA08EB382D69ULL;

struct ullpair {
	uint64_t first;
	uint64_t second;
};

static inline uint64_t unaligned_load64(const uint8_t *p) noexcept
{
	uint64_t result;
	io_memmove( static_cast<void*>(&result), p, sizeof(result) );
	return result;
}

static inline uint32_t unaligned_load32(const uint8_t *p) noexcept
{
	uint32_t result;
	io_memmove( static_cast<void*>(&result), p, sizeof(result) );
	return result;
}

#ifdef IO_IS_LITTLE_ENDIAN
static uint64_t fetch_64(const uint8_t *p) noexcept
{
	return unaligned_load64(p);
}

static uint32_t fetch_32(const uint8_t *p) noexcept
{
	return unaligned_load32(p);
}
#else

static __forceinline uint64_t fetch_64(const uint8_t *p) noexcept
{
	return io_bswap64( unaligned_load64(p) );
}

static __forceinline uint32_t fetch_32(const uint8_t *p) noexcept
{
	return io_bswap32( unaligned_load32(p) );
}
#endif // IO_IS_LITTLE_ENDIAN

#ifdef defined(_MSC_VER) || defined(__ICC)

#ifdef _MSC_VER
#	pragma intrinsic(_rotr64)
#endif

static __forceinline uint64_t ror64(const uint64_t val,const uint32_t shift) noexcept
{
	return _rotr64(val,shift);
}

#else

static __forceinline uint64_t ror64(const uint64_t val,const uint32_t shift) noexcept
{
	// Avoid shifting by 64: doing so yields an undefined result.
	return shift == 0 ? val : ((val >> shift) | (val << (64 - shift) ) );
}

#endif // _MSC_VER

static constexpr inline uint64_t shift_mix(uint64_t val) noexcept
{
	return val ^ (val >> 47);
}

static inline uint64_t hash_len16(uint64_t u, uint64_t v) noexcept
{
	// Murmur-inspired hashing.
	uint64_t a = (u ^ v) * K_MUL;
	a ^= (a >> 47);
	uint64_t b = (v ^ a) * K_MUL;
	b ^= (b >> 47);
	b *= K_MUL;
	return b;
}

// mur
static inline uint64_t hash_len16(const uint64_t u,const uint64_t v,const uint64_t mul) noexcept
{
	// Murmur-inspired hashing.
	uint64_t mur_a = (u ^ v) * mul;
	mur_a ^= (mur_a >> 47);
	uint64_t ret = (v ^ mur_a) * mul;
	ret ^= (ret >> 47);
	ret *= mul;
	return ret;
}

static uint64_t hash_len0_to16(const uint8_t *s, std::size_t len) noexcept
{
	if (len >= 8) {
		uint64_t mul = K2 + len * 2;
		uint64_t a = fetch_64(s) + K2;
		uint64_t b = fetch_64(s + len - 8);
		uint64_t c = ror64(b, 37) * mul + a;
		uint64_t d = (ror64(a, 25) + b) * mul;
		return hash_len16(c, d, mul);
	}
	else if ( len >= 4 ) {
		uint64_t mul = K2 + len * 2;
		uint64_t a = fetch_32(s);
		return hash_len16(len + (a << 3), fetch_32(s + len - 4), mul);
	}
	else if (len > 0) {
		uint8_t a = s[0];
		uint8_t b = s[len >> 1];
		uint8_t c = s[len - 1];
		uint32_t y = static_cast<uint32_t>(a) + (static_cast<uint32_t>(b) << 8);
		uint32_t z = len + (static_cast<uint32_t>(c) << 2);
		return shift_mix(y * K2 ^ z * K0) * K2;
	}
	return K2;
}

static uint64_t hash_len17_to32(const uint8_t *s, size_t len) noexcept
{
	uint64_t mul = K2 + len * 2;
	uint64_t a = fetch_64(s) * K1;
	uint64_t b = fetch_64(s + 8);
	uint64_t c = fetch_64(s + len - 8) * mul;
	uint64_t d = fetch_64(s + len - 16) * K2;
	return hash_len16(ror64(a + b, 43) + ror64(c, 30) + d,
					  a + ror64(b + K2, 18) + c, mul);
}

static ullpair weak_hash_len32_with_seeds(uint64_t w, uint64_t x, uint64_t y, uint64_t z, uint64_t a, uint64_t b) noexcept
{
	a += w;
	b = ror64(b + a + z, 21);
	uint64_t c = a;
	a += x;
	a += y;
	b += ror64(a, 44);
	return { a + z, b + c};
}

static ullpair weak_hash_len32_with_seeds(const uint8_t* s, uint64_t a, uint64_t b) noexcept
{
	return weak_hash_len32_with_seeds(
			   fetch_64(s),
			   fetch_64(s + 8),
			   fetch_64(s + 16),
			   fetch_64(s + 24),
			   a,b);
}

static uint64_t hash_len33_to_64(const uint8_t *s, size_t len) noexcept
{
	uint64_t mul = K2 + len * 2;
	uint64_t a = fetch_64(s) * K2;
	uint64_t b = fetch_64(s + 8);
	uint64_t c = fetch_64(s + len - 24);
	uint64_t d = fetch_64(s + len - 32);
	uint64_t e = fetch_64(s + 16) * K2;
	uint64_t f = fetch_64(s + 24) * 9;
	uint64_t g = fetch_64(s + len - 8);
	uint64_t h = fetch_64(s + len - 16) * mul;
	uint64_t u = ror64(a + g, 43) + (ror64(b, 30) + c) * 9;
	uint64_t v = ((a + g) ^ d) + f + 1;
	uint64_t w = io_bswap64((u + v) * mul) + h;
	uint64_t x = ror64(e + f, 42) + c;
	uint64_t y = (io_bswap64((v + w) * mul) + g) * mul;
	uint64_t z = e + f + c;
	a = io_bswap64((x + z) * mul + y) + b;
	b = shift_mix((z + a) * mul + d + h) * mul;
	return b + x;
}

static uint64_t hash_over_64(const uint8_t* s, std::size_t count) noexcept
{
	// For arrays over 64 bytes we hash the end first, and then as we
	// loop we keep 56 bytes of state: v, w, x, y, and z.
	uint64_t x = fetch_64(s + count - 40);
	uint64_t y = fetch_64(s + count - 16) + fetch_64(s + count - 56);
	uint64_t z = hash_len16(fetch_64(s + count - 48) + count, fetch_64(s + count - 24));
	ullpair v = weak_hash_len32_with_seeds(s + count - 64, count, z);
	ullpair w = weak_hash_len32_with_seeds(s + count - 32, y + K1, x);
	x = x * K1 + fetch_64(s);
	// Decrease len to the nearest multiple of 64, and operate on 64-byte chunks.
	count = (count - 1) & ~static_cast<size_t>(63);
	do {
		x = ror64(x + y + v.first + fetch_64(s + 8), 37) * K1;
		y = ror64(y + v.second + fetch_64(s + 48), 42) * K1;
		x ^= w.second;
		y += v.first + fetch_64(s + 40);
		z = ror64(z + w.first, 33) * K1;
		v = weak_hash_len32_with_seeds(s, v.second * K1, x + w.first);
		w = weak_hash_len32_with_seeds(s + 32, z + w.second, y + fetch_64(s + 16) );
		std::swap(z, x);
		s += 64;
		count -= 64;
	}
	while (0 != count);
	return hash_len16(hash_len16(v.first, w.first) + shift_mix(y) * K1 + z, hash_len16(v.second, w.second) + x);
}

#if defined(__GNUG__) && !defined(__ICC)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"

static uint64_t hash(const uint8_t* s, std::size_t count) noexcept
{
	switch( count ) {
	case 0 ... 16:
		return hash_len0_to16(s, count);
	case 17 ... 32:
		return hash_len17_to32(s, count);
	case 33 ... 64:
		return hash_len33_to_64(s, count);
	default:
		return hash_over_64(s, count);
	}
}

#pragma GCC diagnostic pop

#else // non GCC

static uint64_t hash(const uint8_t* s, std::size_t count) noexcept
{
	if ( count <= 16 )
		return hash_len0_to16(s, count);
	else if ( count <= 32  )
		return hash_len17_to32(s, count);
	else if (count <= 64)
		return hash_len33_to_64(s, count);
	else
		return hash_over_64(s, count);
}

#endif // non GCC

} // namespace cityhash

#endif // IO_CPU_BITS_64

#ifndef IO_CPU_BITS_64

std::size_t IO_PUBLIC_SYMBOL hash_bytes(const uint8_t* bytes, std::size_t count) noexcept
{
	if( io_unlikely(nullptr == bytes || 0 == count ) )
		return 0;
	return murmur3::hash( reinterpret_cast<const uint8_t*>(bytes), count );
}

#else

std::size_t IO_PUBLIC_SYMBOL hash_bytes(const uint8_t* bytes, std::size_t count) noexcept
{
	if( io_unlikely( nullptr == bytes || 0 == count ) )
		return 0;
	return cityhash::hash( bytes, count );
}

#endif // IO_CPU_BITS_64

} // namespace io
