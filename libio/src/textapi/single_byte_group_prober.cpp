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
#include "io/textapi/detail/single_byte_group_prober.hpp"

namespace io {

namespace detail {

s_prober single_byte_group_prober::create(std::error_code& ec) noexcept
{
	s_prober ret;
	const sequence_model* models[NUM_OF_SBCS_PROBERS] = {
		win1256_sequence_model(),
		iso_8859_6_sequence_model(),
		koi8r_sequence_model(),
		win1251_sequence_model(),
		iso_8859_5_sequence_model(),
		win1251_sequence_model(),
		iso_8859_7_sequence_model(),
		win1252_danish_sequence_model(),
		iso_8859_1_danish_sequence_model(),
		iso_8859_15_danish_sequence_model(),
		iso_8859_3_esperanto_sequence_model(),
		win1252_french_sequence_model(),
		iso_8859_1_french_sequence_model(),
		iso_8859_15_french_sequence_model(),
		win1252_german_sequence_model(),
		iso_8859_1_german_sequence_model(),
		win1250_hungarian_sequence_model(),
		iso_8859_2_hungarian_sequence_model(),
		win1252_spanish_sequence_model(),
		iso_8859_1_spanish_sequence_model(),
		iso_8859_15_spanish_sequence_model(),
		iso_8859_3_turkish_sequence_model(),
		iso_8859_9_turkish_sequence_model()
	};
	std::array<s_prober,NUM_OF_SBCS_PROBERS> prbrs;
	for(std::size_t i=0; (i < NUM_OF_SBCS_PROBERS) && !ec; i++) {
		prbrs[i] = single_byte_prober::create(ec, models[i], false );
	}
	if(!ec) {
		single_byte_group_prober* px = new (std::nothrow) single_byte_group_prober( std::move(prbrs) );
		if(nullptr == px)
			ec = std::make_error_code(std::errc::not_enough_memory);
		else
			ret.reset(px, true);
	}
	return ret;
}

single_byte_group_prober::single_byte_group_prober(std::array<s_prober,NUM_OF_SBCS_PROBERS>&& probers) noexcept:
	prober(),
	state_( prober::state_t::detecting ),
	best_guess_prober_(-1),
	active_probers_count_(NUM_OF_SBCS_PROBERS),
	active_({true}),
	probers_(std::forward< std::array<s_prober,NUM_OF_SBCS_PROBERS> >(probers) )
{}

uint16_t single_byte_group_prober::get_charset_code() noexcept
{
	//if we have no answer yet
	if (best_guess_prober_ >= 0) {
		float conf = confidence();
		//no charset seems positive
		if (best_guess_prober_ < 0 || conf < 0.0F)
			//we will use default.
			best_guess_prober_ = 0;
	}
	return probers_[ static_cast<std::size_t>(best_guess_prober_) ] ->get_charset_code();
}

prober::state_t single_byte_group_prober::handle_data(std::error_code& ec, const uint8_t* buff, std::size_t size) noexcept
{
	byte_buffer new_buf = filter_without_english_letters(ec, buff, size);
	if(!ec) {
		prober::state_t	st;
		for(std::size_t i = 0; i < probers_.size(); i++) {
			st = probers_[i]->handle_data( ec, new_buf.position().get(), new_buf.length() );
			if(ec) {
				break;
			}
			else if(prober::state_t::found == st) {
				best_guess_prober_ = static_cast<int>(i);
				state_ = st;
				break;
			}
			else if(prober::state_t::notme == st) {
				active_[i] = false;
				if( --active_probers_count_ <= 0) {
					state_ = st;
					break;
				}
			}
		}
	}
	return state_;
}

prober::state_t single_byte_group_prober::state() noexcept
{
	return state_;
}

void single_byte_group_prober::reset() noexcept
{
	state_ = prober::state_t::detecting;
	active_probers_count_ = NUM_OF_SBCS_PROBERS;
	best_guess_prober_ = -1;
	for(auto it: probers_) {
		it->reset();
	}
}

float single_byte_group_prober::confidence() noexcept
{
	float ret = 0.0F;
	bool look_up_best = true;
	switch (state_) {
	case prober::state_t::found:
		ret = 0.99F;
		look_up_best = false;
		break;
	case prober::state_t::notme:
		ret = 0.01F;
		look_up_best = false;
		break;
	default:
		break;
	}
	if(look_up_best) {
		for(std::size_t i = 0; i < probers_.size(); i++) {
			if(active_[i]) {
				float cnfc =  probers_[i]->confidence();
				if(ret < cnfc) {
					ret = cnfc;
					best_guess_prober_ = static_cast<int>(i);
				}
			}
		}
	}
	return ret;
}

} // namespace detail

} // namespace io
