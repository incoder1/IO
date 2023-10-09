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
#ifndef __IO_TEXT_API_DETAIL_LATIN1_PROBER_HPP_INCLUDED__
#define __IO_TEXT_API_DETAIL_LATIN1_PROBER_HPP_INCLUDED__

#include <io/config/libio_config.hpp>

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <array>

#include "prober.hpp"

namespace io {

namespace detail {

class IO_PUBLIC_SYMBOL latin1_prober final: public prober {
private:
	static constexpr const std::size_t FREQ_CATS_COUNT = 4;
	static constexpr const std::size_t CLASS_NUM = 8;
	static const std::array<uint8_t,256> CHAR_TO_CLASS;
	static const std::array<uint8_t,64> CLASS_MODEL;
	latin1_prober() noexcept;
public:
	static s_prober create(std::error_code& ec) noexcept;
	virtual uint16_t get_charset_code() const noexcept override;
	virtual prober::state_t handle_data(std::error_code& ec, const uint8_t* buff, std::size_t size) noexcept override;
	virtual prober::state_t state() noexcept override;
	virtual void reset() noexcept override;
	virtual float confidence() noexcept override;
private:
	prober::state_t state_;
	std::size_t last_char_class_;
	std::array<std::size_t,FREQ_CATS_COUNT> freq_counter_;
};

} // namespace detail

} // namespace io

#endif // __IO_TEXT_API_DETAIL_LATIN1_PROBER_HPP_INCLUDED__
