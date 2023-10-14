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
#ifndef __IO_TEXT_API_DETAIL_SINGLE_BYTE_GROUP_PROBER_HPP_INCLUDED__
#define __IO_TEXT_API_DETAIL_SINGLE_BYTE_GROUP_PROBER_HPP_INCLUDED__

#include <io/config/libio_config.hpp>

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <array>

#include "single_byte_prober.hpp"
#include "cyrylic_model.hpp"

namespace io {

namespace detail {

class IO_PUBLIC_SYMBOL single_byte_group_prober final: public prober
{
private:
	static constexpr const std::size_t NUM_OF_SBCS_PROBERS = 3;
	single_byte_group_prober(std::array<s_prober,NUM_OF_SBCS_PROBERS>&& probers) noexcept;
public:
	static s_prober create(std::error_code& ec) noexcept;
	virtual uint16_t get_charset_code() noexcept override;
	virtual prober::state_t handle_data(std::error_code& ec, const uint8_t* buff, std::size_t size) noexcept override;
	virtual prober::state_t state() noexcept override;
	virtual void reset() noexcept override;
	virtual float confidence() noexcept override;
public:
	prober::state_t state_;
	int best_guess_prober_;
	std::size_t active_probers_count_;
	bool active_[NUM_OF_SBCS_PROBERS];
	std::array<s_prober,NUM_OF_SBCS_PROBERS> probers_;
};

} // namespace detail

} // namespace io

#endif // __IO_TEXT_API_DETAIL_SINGLE_BYTE_GROUP_PROBER_HPP_INCLUDED__
