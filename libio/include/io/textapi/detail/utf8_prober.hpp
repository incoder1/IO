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
#ifndef __IO_TEXT_API_DETAIL_UTF8_PROBER_HPP_INCLUDED__
#define __IO_TEXT_API_DETAIL_UTF8_PROBER_HPP_INCLUDED__

#include <io/config/libio_config.hpp>

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <memory>

#include "prober.hpp"
#include "coding_state_machine.hpp"

namespace io {

namespace detail {

class IO_PUBLIC_SYMBOL utf8_prober final: public prober {
	utf8_prober(const utf8_prober&) = delete;
	utf8_prober& operator=(utf8_prober&) = delete;
private:
	static constexpr int SHORTCUT_THRESHOLD = 95;
	static constexpr std::size_t MIN_MULTIBYTE_CHARS = 6;
	explicit utf8_prober(std::unique_ptr<coding_state_machine>&& sm) noexcept;
public:
	static s_prober create(std::error_code& ec) noexcept;
	virtual uint16_t get_charset_code() noexcept override;
	virtual prober::state_t handle_data(std::error_code& ec, const uint8_t* buff, std::size_t size) noexcept override;
	virtual prober::state_t state() noexcept override;
	virtual void reset() noexcept override;
	virtual float confidence() noexcept override;
private:
	std::unique_ptr<coding_state_machine> coding_sm_;
	prober::state_t state_;
	std::size_t multibyte_chars_count_;
};

} // namespace detail

} // namespace io

#endif // __IO_TEXT_API_DETAIL_UTF8_PROBER_HPP_INCLUDED__
