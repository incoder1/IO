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
#include "io/textapi/detail/utf8_prober.hpp"

#include <cmath>

namespace io {

namespace detail {

static const uint32_t UTF8_CLS [ 32 ] = {
	pck_4bits( 1, 1, 1, 1, 1, 1, 1, 1),  // 00 - 07
	pck_4bits( 1, 1, 1, 1, 1, 1, 0, 0),  // 08 - 0f
	pck_4bits( 1, 1, 1, 1, 1, 1, 1, 1),  // 10 - 17
	pck_4bits( 1, 1, 1, 0, 1, 1, 1, 1),  // 18 - 1f
	pck_4bits( 1, 1, 1, 1, 1, 1, 1, 1),  // 20 - 27
	pck_4bits( 1, 1, 1, 1, 1, 1, 1, 1),  // 28 - 2f
	pck_4bits( 1, 1, 1, 1, 1, 1, 1, 1),  // 30 - 37
	pck_4bits( 1, 1, 1, 1, 1, 1, 1, 1),  // 38 - 3f
	pck_4bits( 1, 1, 1, 1, 1, 1, 1, 1),  // 40 - 47
	pck_4bits( 1, 1, 1, 1, 1, 1, 1, 1),  // 48 - 4f
	pck_4bits( 1, 1, 1, 1, 1, 1, 1, 1),  // 50 - 57
	pck_4bits( 1, 1, 1, 1, 1, 1, 1, 1),  // 58 - 5f
	pck_4bits( 1, 1, 1, 1, 1, 1, 1, 1),  // 60 - 67
	pck_4bits( 1, 1, 1, 1, 1, 1, 1, 1),  // 68 - 6f
	pck_4bits( 1, 1, 1, 1, 1, 1, 1, 1),  // 70 - 77
	pck_4bits( 1, 1, 1, 1, 1, 1, 1, 1),  // 78 - 7f
	pck_4bits( 2, 2, 2, 2, 2, 2, 2, 2),  // 80 - 87
	pck_4bits( 2, 2, 2, 2, 2, 2, 2, 2),  // 88 - 8f
	pck_4bits( 3, 3, 3, 3, 3, 3, 3, 3),  // 90 - 97
	pck_4bits( 3, 3, 3, 3, 3, 3, 3, 3),  // 98 - 9f
	pck_4bits( 4, 4, 4, 4, 4, 4, 4, 4),  // a0 - a7
	pck_4bits( 4, 4, 4, 4, 4, 4, 4, 4),  // a8 - af
	pck_4bits( 4, 4, 4, 4, 4, 4, 4, 4),  // b0 - b7
	pck_4bits( 4, 4, 4, 4, 4, 4, 4, 4),  // b8 - bf
	pck_4bits( 0, 0, 5, 5, 5, 5, 5, 5),  // c0 - c7
	pck_4bits( 5, 5, 5, 5, 5, 5, 5, 5),  // c8 - cf
	pck_4bits( 5, 5, 5, 5, 5, 5, 5, 5),  // d0 - d7
	pck_4bits( 5, 5, 5, 5, 5, 5, 5, 5),  // d8 - df
	pck_4bits( 6, 7, 7, 7, 7, 7, 7, 7),  // e0 - e7
	pck_4bits( 7, 7, 7, 7, 7, 8, 7, 7),  // e8 - ef
	pck_4bits( 9,10,10,10,11, 0, 0, 0),  // f0 - f7
	pck_4bits( 0, 0, 0, 0, 0, 0, 0, 0)   // f8 - ff
};

static constexpr uint32_t SSTART = static_cast<uint32_t>(coding_state_machine::state_t::start);
static constexpr uint32_t SERROR = static_cast<uint32_t>(coding_state_machine::state_t::error);
static constexpr uint32_t SFOUND = static_cast<uint32_t>(coding_state_machine::state_t::found);

static const uint32_t UTF8_STETES [15] = {
	pck_4bits(SERROR,SSTART,SERROR,SERROR,SERROR,     3,     4,     5),  // 00 - 07
	pck_4bits(     6,     7,     8,     9,SERROR,SERROR,SERROR,SERROR),  // 08 - 0f
	pck_4bits(SERROR,SERROR,SERROR,SERROR,SERROR,SERROR,SERROR,SERROR),  // 10 - 17
	pck_4bits(SFOUND,SFOUND,SFOUND,SFOUND,SFOUND,SFOUND,SFOUND,SFOUND),  // 18 - 1f
	pck_4bits(SFOUND,SFOUND,SFOUND,SFOUND,SERROR,SERROR,SSTART,SSTART),  // 20 - 27
	pck_4bits(SSTART,SERROR,SERROR,SERROR,SERROR,SERROR,SERROR,SERROR),  // 28 - 2f
	pck_4bits(SERROR,SERROR,SERROR,SERROR,     3,SERROR,SERROR,SERROR),  // 30 - 37
	pck_4bits(SERROR,SERROR,SERROR,SERROR,SERROR,SERROR,     3,     3),  // 38 - 3f
	pck_4bits(     3,SERROR,SERROR,SERROR,SERROR,SERROR,SERROR,SERROR),  // 40 - 47
	pck_4bits(SERROR,SERROR,     3,     3,SERROR,SERROR,SERROR,SERROR),  // 48 - 4f
	pck_4bits(SERROR,SERROR,SERROR,SERROR,SERROR,SERROR,     5,     5),  // 50 - 57
	pck_4bits(SERROR,SERROR,SERROR,SERROR,SERROR,SERROR,SERROR,SERROR),  // 58 - 5f
	pck_4bits(SERROR,SERROR,     5,     5,     5,SERROR,SERROR,SERROR),  // 60 - 67
	pck_4bits(SERROR,SERROR,SERROR,SERROR,SERROR,SERROR,     5,SERROR),  // 68 - 6f
	pck_4bits(SERROR,SERROR,SERROR,SERROR,SERROR,SERROR,SERROR,SERROR)   // 70 - 77
};

static const uint32_t UTF8_CHAR_LEN_TABLE[] = {0, 1, 0, 0, 0, 2, 3, 3, 3, 4, 4, 4};

#ifndef NDEBUG
#	define CHAR_LEN_TABLE(tbl) tbl, ( sizeof(tbl) / sizeof(uint32_t) )
#else
#	define CHAR_LEN_TABLE(tbl) tbl
#endif // NDEBUG

static const coding_state_machine::model_t UTF8_MODEL = {
	{
		index_shift::bits_4,
		shift_mask::bits_4,
		bit_shift::bits_4,
		unit_mask::bits_4,
		UTF8_CLS
	},
	12,
	{
		index_shift::bits_4,
		shift_mask::bits_4,
		bit_shift::bits_4,
		unit_mask::bits_4,
		UTF8_STETES
	},
	CHAR_LEN_TABLE(UTF8_CHAR_LEN_TABLE),
	65001,
};

#undef CHAR_LEN_TABLE

//utf8_prober
s_prober utf8_prober::create(std::error_code& ec) noexcept
{
	s_prober ret;
	coding_state_machine* smpx = new (std::nothrow) coding_state_machine( &UTF8_MODEL );
	if(nullptr == smpx) {
		ec = std::make_error_code( std::errc::not_enough_memory );
	}
	else {
		utf8_prober* px = new (std::nothrow) utf8_prober( std::unique_ptr<coding_state_machine>(smpx) );
		if(nullptr == px)
			ec = std::make_error_code( std::errc::not_enough_memory );
		else
			ret.reset(px,true);
	}
	return ret;
}

utf8_prober::utf8_prober(std::unique_ptr<coding_state_machine>&& sm) noexcept:
	coding_sm_(std::forward< std::unique_ptr<coding_state_machine> > (sm) ),
	state_(prober::state_t::detecting),
	multibyte_chars_count_(0)
{}

uint16_t utf8_prober::get_charset_code() const noexcept
{
	return 65001;
}

prober::state_t utf8_prober::handle_data(std::error_code& ec, const uint8_t* buff, std::size_t size) noexcept
{
	coding_state_machine::state_t coding_state;
	for(std::size_t i = 0; i < size; i++) {
		coding_state = coding_sm_->next_state(buff[i]);
		switch(coding_state) {
		case coding_state_machine::state_t::found:
			state_ = prober::state_t::found;
			break;
		case coding_state_machine::state_t::start:
			if (coding_sm_->current_char_len() >= 2)
        		++multibyte_chars_count_;
			continue;
		default:
			continue;
		}
	}

	if( prober::state_t::detecting == state_  ) {
		if(multibyte_chars_count_ < MIN_MULTIBYTE_CHARS) {
			int percent = std::truncf( confidence() * 100.0F);
			if( percent > SHORTCUT_THRESHOLD )
				state_ = prober::state_t::found;
		}
		else
			state_ = prober::state_t::found;
	}

	return state_;
}

prober::state_t utf8_prober::state() noexcept
{
	return state_;
}

void utf8_prober::reset() noexcept
{
	coding_sm_->reset();
	multibyte_chars_count_ = 0;
	state_ = prober::state_t::detecting;
}

float utf8_prober::confidence() noexcept
{
	float ret = 0.99F;
	if (multibyte_chars_count_ < MIN_MULTIBYTE_CHARS) {
		for (uint32_t i = 0; i < multibyte_chars_count_; i++) {
			ret = ret * 0.5F;
		}
		ret = 1.0F - ret;
	}
	return ret;
}

} // namespace detail

} // namespace io
