/*
 *
 * Copyright (c) 2016
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef __IO_CHARSETS_CVT_HPP_INCLUDED__
#define __IO_CHARSETS_CVT_HPP_INCLUDED__

#include <cerrno>
#include <functional>
#include <system_error>
#include <locale>

#include "buffer.hpp"
#include "charsets.hpp"
#include "object.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#ifndef iconv_t
#define iconv_t libiconv_t
typedef void* iconv_t;
#endif // iconv_t

namespace io {

/// \brief Xharacter set conversation (transcoding) error code
enum converrc: int {
	/// conversation was success
	success = 0,

	/// character code conversion invalid multi-byte sequence
	invalid_multibyte_sequence = 1,

	/// character code conversion incomplete multi-byte sequence
	incomplete_multibyte_sequence = 2,

	/// character code conversion no more room i.e. output buffer overflow
	no_buffer_space = 3,

	/// conversation between character's is not supported
	not_supported,

	/// An unknown error
	unknown = -1
};

class IO_PUBLIC_SYMBOL chconv_error_category final: public std::error_category {
private:
	friend std::error_code  make_error_code(io::converrc ec) noexcept;
	friend std::error_condition  make_error_condition(io::converrc err) noexcept;
	static inline const chconv_error_category* instance()
	{
		static chconv_error_category _instance;
		return &_instance;
	}

	const char* cstr_message(int err_code) const;

public:

	constexpr chconv_error_category() noexcept:
		error_category()
	{}

	virtual ~chconv_error_category() = default;

	virtual const char* name() const noexcept override;

	virtual std::error_condition default_error_condition (int err) const noexcept override;

	virtual bool equivalent (const std::error_code& code, int condition) const noexcept override;

	virtual std::string message(int err_code) const override
	{
		return std::string(  cstr_message(err_code) );
	}

};

std::error_code IO_PUBLIC_SYMBOL make_error_code(io::converrc errc) noexcept;
std::error_condition  IO_PUBLIC_SYMBOL make_error_condition(io::converrc err) noexcept;

enum class cnvrt_control
{
	failure_on_failing_chars,
	discard_on_failing_chars
};


namespace detail {

class engine {
public:
	engine(const engine&) = delete;
	engine& operator=(const engine&) = delete;

	engine(engine&& other) noexcept;
	engine& operator=(engine&& rhs) noexcept;
	operator bool() const
	{
		return is_open();
	}

	engine() noexcept;
	engine(const char* from,const char* to, cnvrt_control control) noexcept;
	~engine() noexcept;

	inline void swap(engine& other) noexcept;
	bool is_open() const;

	converrc convert(uint8_t** src,std::size_t& size, uint8_t** dst, std::size_t& avail) const noexcept;
private:
	iconv_t iconv_;
};

} // namespace detail

class IO_PUBLIC_SYMBOL code_cnvtr;
DECLARE_IPTR(code_cnvtr);

/// \brief character set conversation (transcoding) interface
class IO_PUBLIC_SYMBOL code_cnvtr final: public object {
public:
	/// Cursor character transcoding API, direct access to conversation engine
	/// \param ec
	///    operation error code
	/// \param in
	///    source block of input character sequence, as raw bytes
	/// \param in_bytes_left
	///
	/// \param out
	void convert(std::error_code& ec, uint8_t** in,std::size_t& in_bytes_left,uint8_t** const out, std::size_t& out_bytes_left) const noexcept;

	void convert(std::error_code& ec, const uint8_t* src,const std::size_t src_size, byte_buffer& dst) const noexcept;

private:
	code_cnvtr(detail::engine&& eng) noexcept;
	friend class io::nobadalloc<code_cnvtr>;
public:
	static s_code_cnvtr open(std::error_code& ec,const charset& from,const charset& to, cnvrt_control conrol) noexcept;
	static std::error_condition _ok;
private:
	detail::engine eng_; // converting engine
};

/// Convert a character array UTF-8 encoded to platform current USC-2 (UTF-16LE or UTF-16BE) character array
/// \param  ec operation error code
/// \param  u8_scr source character array UTF-8 encoded, must not be nullptr and at least src_bytes whide
/// \param  src_bytes size of source array in bytes
/// \param  dst destination memory buffer, must not be nullptr and at least dst_size elements whide
/// \param  dst_size size of destination memory buffer available elements in char16_t elements
/// \return count char16_t elements in destination array, or 0 in case of failure
/// \throw never throws
std::size_t IO_PUBLIC_SYMBOL transcode(std::error_code& ec,const uint8_t* u8_src, std::size_t src_bytes, char16_t* const u16_dst, std::size_t dst_size) noexcept;

/// Convert a character array UTF-8 encoded to platform current UCS-4 (UTF-32LE or UTF-32BE) character array
/// \param  ec operation error code
/// \param  u8_src source character array UTF-8 encoded
/// \param  src_bytes size of source array in bytes
/// \param  u32_dst destination memory buffer, must not be nullptr and at least dst_size width
/// \param  dst_size size of destination memory buffer available char32_t elements
/// \return count of filled char32_t elements in destinaton array, or 0 in case of failure
/// \throw never throws
std::size_t IO_PUBLIC_SYMBOL transcode(std::error_code& ec,const uint8_t* u8_src, std::size_t src_bytes, char32_t* const u32_dst, std::size_t dst_size) noexcept;

/// Converts platform current UCS-2 (UTF-16LE or UTF-16BE) character array to UTF-8 encoded character array
/// \param  ec operation error code
/// \param  u16_src source character array UCS-2 encoded must not be nullptr and at least src_width whide
/// \param  src_width size of source array in char16_t elements
/// \param  dst destination memory buffer, must not be nullptr and at least dst_size bytes whide
/// \param  dst_size size of destination memory buffer available char16_t elements
/// \return count of filled bytes in destinaton character array, or 0 in case of failure
/// \throw never throws
std::size_t IO_PUBLIC_SYMBOL transcode(std::error_code& ec,const char16_t* u16_src, std::size_t src_width, uint8_t* const u8_dst, std::size_t dst_size) noexcept;


/// Converts platform current UCS-4 (UTF-32LE or UTF-32BE) character array to UTF-8 encoded character array
/// \param  ec operation error code
/// \param  u32_src source character array UCS-4 encoded
/// \param  src_width size of source array in char32_t elements
/// \param  dst destination memory buffer, must not be nullptr and at least dst_size whide
/// \param  dst_size size of destination memory buffer available elements in char32_t elements
/// \return count of filled bytes in destinaton character array, or 0 in case of failure
/// \throw never throws
std::size_t IO_PUBLIC_SYMBOL transcode(std::error_code& ec,const char32_t* u32_src, std::size_t src_width, uint8_t* const u8_dst, std::size_t dst_size) noexcept;

} // namespace io

namespace std {

template<>
struct is_error_condition_enum<io::converrc> : public true_type
{};

} // namespace std

#endif // __IO_CHARSETS_CVT_HPP_INCLUDED__
