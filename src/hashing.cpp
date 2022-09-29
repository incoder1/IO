/*
 *
 * Copyright (c) 2016-2022
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

// Original hash function can be found https://github.com/google/cityhash
namespace cityhash {

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


// CityHash64
#ifdef IO_CPU_BITS_64

static constexpr uint64_t K0 = 0xC3A5C85C97CB3127ULL;
static constexpr uint64_t K1 = 0xB492B66FBE98F273ULL;
static constexpr uint64_t K2 = 0x9AE16A3B2F90404FULL;
static constexpr uint64_t K_MUL = 0x9DDFEA08EB382D69ULL;

constexpr const std::size_t MURMUR_C1 = 47;

static constexpr inline uint64_t shift_mix(uint64_t val) noexcept
{
	return val ^ (val >> MURMUR_C1);
}

static inline uint64_t hash_len16(uint64_t u, uint64_t v) noexcept
{
	// Murmur-inspired hashing.
	uint64_t a = (u ^ v) * K_MUL;
	a ^= (a >> MURMUR_C1);
	uint64_t b = (v ^ a) * K_MUL;
	b ^= (b >> MURMUR_C1);
	b *= K_MUL;
	return b;
}

// mur
static inline uint64_t hash_len16(const uint64_t u,const uint64_t v,const uint64_t mul) noexcept
{
	// Murmur-inspired hashing.
	uint64_t mur_a = (u ^ v) * mul;
	mur_a ^= (mur_a >> MURMUR_C1);
	uint64_t ret = (v ^ mur_a) * mul;
	ret ^= (ret >> MURMUR_C1);
	ret *= mul;
	return ret;
}

static uint64_t hash_0_to_16(const uint8_t *s, uint32_t len) noexcept
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
		uint32_t a = static_cast<uint32_t>(s[0]);
		uint32_t b = static_cast<uint32_t>(s[len >> 1]);
		uint32_t c = static_cast<uint32_t>(s[len - 1]);
		uint32_t y = a + (b << 8);
		uint32_t z = len + (c << 2);
		return shift_mix(y * K2 ^ z * K0) * K2;
	}
	return K2;
}

static uint64_t hash_17_to_32(const uint8_t *s, uint32_t len) noexcept
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
					a,
					b);
}

static uint64_t hash_33_to_64(const uint8_t *s, uint32_t len) noexcept
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
	uint64_t u = ror64( (a + g), 43) + (ror64(b, 30) + c) * 9;
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
	const uint8_t *p = s + count;
	uint64_t x = fetch_64( p - 40);
	uint64_t y = fetch_64( p - 16) + fetch_64( p - 56);
	uint64_t z = hash_len16( (fetch_64(p - 48) + count), fetch_64(p - 24) );
	ullpair v = weak_hash_len32_with_seeds( (p - 64), count, z);
	ullpair w = weak_hash_len32_with_seeds( (p - 32), (y + K1), x);
	x = (x * K1) + fetch_64(s);
	// Decrease len to the nearest multiple of 64, and operate on 64-byte chunks.
	count = (count - 1) & ~static_cast<std::size_t>(63);
	do {
		x = ror64( (x + y + v.first + fetch_64(s + 8)), 37) * K1;
		x ^= w.second;
		y = ror64( (y + v.second + fetch_64(s + 48)), 42) * K1;
		y += v.first + fetch_64(s + 40);
		z = ror64( (z + w.first), 33) * K1;
		v = weak_hash_len32_with_seeds(s, (v.second * K1), (x + w.first) );
		w = weak_hash_len32_with_seeds( (s + 32), (z + w.second), (y + fetch_64(s + 16)) );
		std::swap(z, x);
		s += 64;
		count -= 64;
	}
	while (0 != count);
	return hash_len16(hash_len16(v.first, w.first) + shift_mix(y) * K1 + z, hash_len16(v.second, w.second) + x);
}

static uint64_t hash(const uint8_t* s, std::size_t count) noexcept
{
#if defined(__GNUG__) && !defined(__ICC)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
	switch( count ) {
	case 0 ... 16:
		return hash_0_to_16(s, static_cast<uint32_t>(count) );
	case 17 ... 32:
		return hash_17_to_32(s, static_cast<uint32_t>(count) );
	case 33 ... 64:
		return hash_33_to_64(s, static_cast<uint32_t>(count) );
	default:
		return hash_over_64(s, count);
	}
#pragma GCC diagnostic pop

#else // non GCC
	if ( count <= 16 )
		return hash_0_to_16(s, static_cast<uint32_t>(count) );
	else if ( count <= 32  )
		return hash_17_to_32(s, static_cast<uint32_t>(count) );
	else if (count <= 64)
		return hash_33_to_64(s, static_cast<uint32_t>(count) );
	else
		return hash_over_64(s, count);
#endif // non GCC
}

static uint64_t combine(const uint64_t seed,const uint64_t value) noexcept
{
	return hash_len16(seed, value);
}

#endif // IO_CPU_BITS_64

// CityHash32
#ifdef IO_CPU_BITS_32

// Magic numbers for 32-bit hashing.  Copied from Murmur3.
static constexpr const uint32_t MURMUR_C1 = 0xcc9e2d51;
static constexpr const uint32_t MURMUR_C2 = 0x1b873593;
static constexpr uint32_t PRED = 0xe6546b64;


static uint32_t fmix(uint32_t h)
{
	h ^= h >> 16;
	h *= 0x85ebca6b;
	h ^= h >> 13;
	h *= 0xc2b2ae35;
	h ^= h >> 16;
	return h;
}

// Helper from Murmur3 for combining two 32-bit values.
static uint32_t mur(uint32_t a, uint16_t h) noexcept
{
	a *= MURMUR_C1;
	a = ror32(a, 17);
	a *= MURMUR_C2;
	h ^= a;
	h = ror32(h, 19);
	return h * 5 + PRED;
}

static uint32_t hash_0_to_4(const char *s, std::size_t len) noexcept
{
	uint32_t b = 0;
	uint32_t c = 9;
	for (size_t i = 0; i < len; i++) {
		uint8_t v = s[i];
		b = b * MURMUR_C1 + v;
		c ^= b;
	}
	return fmix(mur(b, mur(len, c)));
}

static uint32_t hash_5_to_12(const uint8_t *s, std::size_t len) noexcept
{
	uint32_t a = len, b = len * 5, c = 9, d = b;
	a += fetch_32(s);
	b += fetch_32(s + len - 4);
	c += fetch_32(s + ((len >> 1) & 4));
	return fmix(mur(c, mur(b, mur(a, d))));
}

static uint32_t hash_13_to_24(const uint8_t *s, std::size_t len) noexcept
{
	uint32_t a = fetch_32(s - 4 + (len >> 1));
	uint32_t b = fetch_32(s + 4);
	uint32_t c = fetch_32(s + len - 8);
	uint32_t d = fetch_32(s + (len >> 1));
	uint32_t e = fetch_32(s);
	uint32_t f = fetch_32(s + len - 4);
	uint32_t h = len;
	return fmix(mur(f, mur(e, mur(d, mur(c, mur(b, mur(a, h)))))));
}

static void permute3(uint32_t& a, uint32_t& b, uint32_t& c)
{
	std::swap(a, b);
	std::swap(a, c);
}

static uint32_t hash_over_24(const uint8_t* s, std::size_t len) noexcept
{
	uint32_t h = len;
	uint32_t g = MURMUR_C1 * len;
	uint32_t f = g;

	uint32_t a0 = ror32(fetch_32(s + len - 4) * MURMUR_C1, 17) * MURMUR_C2;
	uint32_t a1 = ror32(fetch_32(s + len - 8) * MURMUR_C1, 17) * MURMUR_C2;
	uint32_t a2 = ror32(fetch_32(s + len - 16) * MURMUR_C1, 17) * MURMUR_C2;
	uint32_t a3 = ror32(fetch_32(s + len - 12) * MURMUR_C1, 17) * MURMUR_C2;
	uint32_t a4 = ror32(fetch_32(s + len - 20) * MURMUR_C1, 17) * MURMUR_C2;

	h ^= a0;
	h = ror32(h, 19);
	h = h * 5 + PRED;
	h ^= a2;
	h = ror32(h, 19);
	h = h * 5 + PRED;
	g ^= a1;
	g = ror32(g, 19);
	g = g * 5 + PRED;
	g ^= a3;
	g = ror32(g, 19);
	g = g * 5 + PRED;
	f += a4;
	f = ror32(f, 19);
	f = f * 5 + PRED;
	size_t iters = (len - 1) / 20;
	do {
		uint32_t a0 = ror32(fetch_32(s) * MURMUR_C1, 17) * MURMUR_C2;
		uint32_t a1 = fetch_32(s + 4);
		uint32_t a2 = ror32(fetch_32(s + 8) * MURMUR_C1, 17) * MURMUR_C2;
		uint32_t a3 = ror32(fetch_32(s + 12) * MURMUR_C1, 17) * MURMUR_C2;
		uint32_t a4 = fetch_32(s + 16);
		h ^= a0;
		h = ror32(h, 18);
		h = h * 5 + PRED;
		f += a1;
		f = ror32(f, 19);
		f = f * MURMUR_C1;
		g += a2;
		g = ror32(g, 18);
		g = g * 5 + PRED;
		h ^= a3 + a1;
		h = ror32(h, 19);
		h = h * 5 + PRED;
		g ^= a4;
		g = io_bswap32(g) * 5;
		h += a4 * 5;
		h = io_bswap32(h);
		f += a0;
		permute3(f,h,g);
		s += 20;
	}
	while (--iters != 0);
	g = ror32(g, 11) * MURMUR_C1;
	g = ror32(g, 17) * MURMUR_C1;
	f = ror32(f, 11) * MURMUR_C1;
	f = ror32(f, 17) * MURMUR_C1;
	h = ror32(h + g, 19);
	h = h * 5 + PRED;
	h = ror32(h, 17) * MURMUR_C1;
	h = ror32(h + f, 19);
	h = h * 5 + PRED;
	h = ror32(h, 17) * MURMUR_C1;
	return h;
}

static uint32_t hash(const uint8_t *s, std::size_t len) noexcept
{
	if (len <= 24) {
		return len <= 12 ?
				(len <= 4 ? hash_0_to_4(s, len) : hash_5_to_12(s, len)) :
				hash_13_to_24(s, len);
	}
	return hash_over_24(s,len);
}

static uint32_t combine(uint32_t seed, uint32_t value) noexcept
{
	return mur(seed, value);
}

#endif // City32

} // namespace cityhash


void IO_PUBLIC_SYMBOL hash_combine(std::size_t& seed,const std::size_t value) noexcept
{
	seed = cityhash::combine(seed,value);
}

std::size_t IO_PUBLIC_SYMBOL hash_bytes(const uint8_t* bytes, std::size_t count) noexcept
{
	if( io_unlikely( nullptr == bytes || 0 == count ) )
		return 0;
	return cityhash::hash( bytes, count );
}

} // namespace io
