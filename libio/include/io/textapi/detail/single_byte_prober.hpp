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
#ifndef __IO_TEXT_API_DETAIL_SINGLE_BYTETE_CHARSET_PROBER_HPP_INCLUDED__
#define __IO_TEXT_API_DETAIL_SINGLE_BYTETE_CHARSET_PROBER_HPP_INCLUDED__

#include <io/config/libio_config.hpp>

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <array>

#include "prober.hpp"

namespace io {

namespace detail {

constexpr const uint8_t ILL = 0xFF;
constexpr const uint8_t CTR = 0xFE;
constexpr const uint8_t SYM = 0xFD;
constexpr const uint8_t RET = 0xFC;
constexpr const uint8_t NUM = 0xFB;

struct sequence_model
{
	const uint8_t* const char_to_order_map;// [256] table use to find a char's order
	const uint8_t* const precedence_matrix;// [SAMPLE_SIZE][SAMPLE_SIZE]; table to find a 2-char sequence's frequency
	const float typical_positive_ratio;// = freqSeqs / totalSeqs
	const uint16_t charset_code;
};

class IO_PUBLIC_SYMBOL single_byte_prober final: public prober {
	single_byte_prober(single_byte_prober&) = delete;
	single_byte_prober& operator=(single_byte_prober&) = delete;
private:
	static constexpr const std::size_t NUMBER_OF_SEQ_CAT = 4;
	static constexpr const std::size_t POSITIVE_CAT = (NUMBER_OF_SEQ_CAT-1);
	static constexpr const std::size_t NEGATIVE_CAT = 0;
	static constexpr const uint32_t SB_ENOUGH_REL_THRESHOLD	= 1024;
	static constexpr const uint32_t POSITIVE_SHORTCUT_THRESHOLD = 95;
	static constexpr const uint32_t NEGATIVE_SHORTCUT_THRESHOLD = 5;
	static constexpr const uint8_t SAMPLE_SIZE = 64;
	static constexpr const uint8_t SYMBOL_CAT_ORDER = 250;

	single_byte_prober(const sequence_model* model, bool reversed) noexcept;

public:
	static s_prober create(std::error_code& ec, const sequence_model* model, bool reversed) noexcept;
	virtual uint16_t get_charset_code() noexcept override;
	virtual prober::state_t handle_data(std::error_code& ec, const uint8_t* buff, std::size_t size) noexcept override;
	virtual prober::state_t state() noexcept override;
	virtual void reset() noexcept override;
	virtual float confidence() noexcept override;
private:
	const sequence_model* const model_;
	const bool reversed_;

	prober::state_t state_;
	std::size_t total_seqs_;
	std::size_t total_char_;
	std::size_t freq_char_;
	uint8_t last_order_;

	std::array<uint32_t, NUMBER_OF_SEQ_CAT> seq_counters_;
};


}	// namespace detail

} // namespace io

#endif // __IO_TEXT_API_DETAIL_SINGLE_BYTETE_CHARSET_PROBER_HPP_INCLUDED__
