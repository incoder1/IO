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
#ifndef __IO_TEXT_API_DETAIL_PROBER_HPP_INCLUDED__
#define __IO_TEXT_API_DETAIL_PROBER_HPP_INCLUDED__

#include <io/config/libio_config.hpp>

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <system_error>

#include <io/core/buffer.hpp>

namespace io {

namespace detail {

class prober;
DECLARE_IPTR(prober);

class IO_PUBLIC_SYMBOL prober: public virtual io::object {
	prober(const prober&) = delete;
	prober& operator=(const prober&) = delete;
private:
	static const char* ENGLISH_LETTERS;
protected:
	prober() noexcept;
public:

	enum class state_t {
		detecting = 0, // We are still detecting, no sure answer yet, but caller can ask for confidence.
		found = 1,	 // That's a positive answer
		notme = 2	  //  Negative answer
	};

	virtual uint16_t get_charset_code() const noexcept = 0;
	virtual state_t handle_data(std::error_code& ec, const uint8_t* buff, std::size_t size) noexcept = 0;

	virtual state_t state() noexcept = 0;
	virtual void reset() noexcept = 0;
	virtual float confidence() noexcept = 0;

	//This filter applies to all scripts which do not use English characters
	static io::byte_buffer filter_without_english_letters(std::error_code& ec,const uint8_t* buff, std::size_t size) noexcept;
	//This filter applies to all scripts which contain both English characters and upper ASCII characters
	static io::byte_buffer filter_with_english_letters(std::error_code& ec,const uint8_t* buff, std::size_t size) noexcept;
};

} // namespace detail

} // namespace io


#endif // __IO_TEXT_API_DETAIL_PROBER_HPP_INCLUDED__
