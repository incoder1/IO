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
#include "stdafx.hpp"
#include "io/textapi/detail/single_byte_prober.hpp"

#include <cmath>

namespace io {

namespace detail {


// single_byte_prober
s_prober single_byte_prober::create(std::error_code& ec, const sequence_model* model, bool reversed) noexcept
{
	s_prober ret;
	single_byte_prober *px = new (std::nothrow) single_byte_prober(model, reversed);
	if(nullptr == px)
		ec = std::make_error_code(std::errc::not_enough_memory);
	else
		ret.reset(px, true);
	return ret;
}

single_byte_prober::single_byte_prober(const sequence_model* model, bool reversed) noexcept:
	prober(),
	model_(model),
	reversed_(reversed),
	state_( prober::state_t::detecting ),
	total_seqs_(0),
	total_char_(0),
	freq_char_(0),
	last_order_( std::numeric_limits<uint8_t>::max() ),
	seq_counters_()
{
	std::fill(seq_counters_.begin(),seq_counters_.end(),0);
}

uint16_t single_byte_prober::get_charset_code() noexcept
{
	return model_->charset_code;
}

prober::state_t single_byte_prober::handle_data(std::error_code& ec, const uint8_t* buff, std::size_t size) noexcept
{
	for(std::size_t i = 0; i < size; i++) {
		uint8_t order = model_->char_to_order_map[ static_cast<std::size_t>( buff[i] ) ];
		if (order < SYMBOL_CAT_ORDER)
			++total_char_;
		if (order < SAMPLE_SIZE) {
			++freq_char_;
			if (last_order_ < SAMPLE_SIZE) {
				++total_seqs_;
				std::size_t pxid;
				if (reversed_)
					pxid = static_cast<std::size_t>(order) * SAMPLE_SIZE + last_order_;
				else
					pxid = last_order_ * SAMPLE_SIZE + static_cast<std::size_t>(order);
				std::size_t cixid = model_->precedence_matrix[pxid];
				seq_counters_[cixid] += 1;
			}
		}
		last_order_ = order;
	}
	if( prober::state_t::detecting == state_ ) {
		if( total_seqs_ > SB_ENOUGH_REL_THRESHOLD) {
			uint32_t confidence_percent = std::truncf( confidence() * 100.0F );
			if( confidence_percent > POSITIVE_SHORTCUT_THRESHOLD ) {
				state_ = prober::state_t::found;
			} else if( confidence_percent < NEGATIVE_SHORTCUT_THRESHOLD ) {
				state_ = prober::state_t::notme;
			}
		}
	}
	return state_;
}

prober::state_t single_byte_prober::state() noexcept
{
	return state_;
}

void single_byte_prober::reset() noexcept
{
	state_ = prober::state_t::detecting;
	total_seqs_ = 0;
	total_char_ = 0;
	freq_char_ = 0;
	last_order_ = std::numeric_limits<uint8_t>::max();
	std::fill(seq_counters_.begin(),seq_counters_.end(), 0);
}

float single_byte_prober::confidence() noexcept
{
	double ret = 0.01;
	if( total_seqs_ > 0) {
		double positive = 1.0 * ( (static_cast<double>(seq_counters_[POSITIVE_CAT])  /  static_cast<double>(total_seqs_)  / static_cast<double>( model_->typical_positive_ratio) ) );
		positive = positive * ( static_cast<double>(freq_char_) / static_cast<double>(total_char_) );
		double negative = std::abs( (static_cast<double>(total_seqs_) - (static_cast<double>(seq_counters_[NEGATIVE_CAT])*10)) / ( static_cast<double>(total_seqs_) * ( static_cast<double>(freq_char_) / static_cast<double>(total_char_) ) ) );
		ret = (positive + negative) / 2.0F;
		if(ret >= 1.0)
			ret = 0.99;
	}
	return static_cast<float>( ret );
}

}  // namespace detail

} // namespace io
