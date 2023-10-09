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
#include "io/textapi/detail/latin1_prober.hpp"

#include <numeric>

namespace io {

namespace detail {

// undefine
static constexpr const uint8_t UDF = 0;
//other
static constexpr const uint8_t OTH = 1;
// ascii capital letter
static constexpr const uint8_t ASC = 2;
// ascii small letter
static constexpr const uint8_t ASS = 3;
// accent capital vowel
static constexpr const uint8_t ACV = 4;
// accent capital other
static constexpr const uint8_t ACO = 5;
// accent small vowel
static constexpr const uint8_t ASV = 6;
// accent small other
static constexpr const uint8_t ASO = 7;

// Latin1Prober
const std::array<uint8_t,256> latin1_prober::CHAR_TO_CLASS = {
	OTH, OTH, OTH, OTH, OTH, OTH, OTH, OTH,	// 00 - 07
	OTH, OTH, OTH, OTH, OTH, OTH, OTH, OTH,	// 08 - 0F
	OTH, OTH, OTH, OTH, OTH, OTH, OTH, OTH,	// 10 - 17
	OTH, OTH, OTH, OTH, OTH, OTH, OTH, OTH,	// 18 - 1F
	OTH, OTH, OTH, OTH, OTH, OTH, OTH, OTH,	// 20 - 27
	OTH, OTH, OTH, OTH, OTH, OTH, OTH, OTH,	// 28 - 2F
	OTH, OTH, OTH, OTH, OTH, OTH, OTH, OTH,	// 30 - 37
	OTH, OTH, OTH, OTH, OTH, OTH, OTH, OTH,	// 38 - 3F
	OTH, ASC, ASC, ASC, ASC, ASC, ASC, ASC,	// 40 - 47
	ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC,	// 48 - 4F
	ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC,	// 50 - 57
	ASC, ASC, ASC, OTH, OTH, OTH, OTH, OTH,	// 58 - 5F
	OTH, ASS, ASS, ASS, ASS, ASS, ASS, ASS,	// 60 - 67
	ASS, ASS, ASS, ASS, ASS, ASS, ASS, ASS,	// 68 - 6F
	ASS, ASS, ASS, ASS, ASS, ASS, ASS, ASS,	// 70 - 77
	ASS, ASS, ASS, OTH, OTH, OTH, OTH, OTH,	// 78 - 7F
	OTH, UDF, OTH, ASO, OTH, OTH, OTH, OTH,	// 80 - 87
	OTH, OTH, ACO, OTH, ACO, UDF, ACO, UDF,	// 88 - 8F
	UDF, OTH, OTH, OTH, OTH, OTH, OTH, OTH,	// 90 - 97
	OTH, OTH, ASO, OTH, ASO, UDF, ASO, ACO,	// 98 - 9F
	OTH, OTH, OTH, OTH, OTH, OTH, OTH, OTH,	// A0 - A7
	OTH, OTH, OTH, OTH, OTH, OTH, OTH, OTH,	// A8 - AF
	OTH, OTH, OTH, OTH, OTH, OTH, OTH, OTH,	// B0 - B7
	OTH, OTH, OTH, OTH, OTH, OTH, OTH, OTH,	// B8 - BF
	ACV, ACV, ACV, ACV, ACV, ACV, ACO, ACO,	// C0 - C7
	ACV, ACV, ACV, ACV, ACV, ACV, ACV, ACV,	// C8 - CF
	ACO, ACO, ACV, ACV, ACV, ACV, ACV, OTH,	// D0 - D7
	ACV, ACV, ACV, ACV, ACV, ACO, ACO, ACO,	// D8 - DF
	ASV, ASV, ASV, ASV, ASV, ASV, ASO, ASO,	// E0 - E7
	ASV, ASV, ASV, ASV, ASV, ASV, ASV, ASV,	// E8 - EF
	ASO, ASO, ASV, ASV, ASV, ASV, ASV, OTH,	// F0 - F7
	ASV, ASV, ASV, ASV, ASV, ASO, ASO, ASO	// F8 - FF
};

const std::array<uint8_t,64> latin1_prober::CLASS_MODEL = {
	/*	  UDF OTH ASC ASS ACV ACO ASV ASO  */
	/*UDF*/  0,  0,  0,  0,  0,  0,  0,  0,
	/*OTH*/  0,  3,  3,  3,  3,  3,  3,  3,
	/*ASC*/  0,  3,  3,  3,  3,  3,  3,  3,
	/*ASS*/  0,  3,  3,  3,  1,  1,  3,  3,
	/*ACV*/  0,  3,  3,  3,  1,  2,  1,  2,
	/*ACO*/  0,  3,  3,  3,  3,  3,  3,  3,
	/*ASV*/  0,  3,  1,  3,  1,  1,  1,  3,
	/*ASO*/  0,  3,  1,  3,  1,  1,  3,  3,
};

s_prober latin1_prober::create(std::error_code& ec) noexcept
{
	s_prober ret;
	latin1_prober* px = new (std::nothrow) latin1_prober();
	if( nullptr == px )
		ec = std::make_error_code(std::errc::not_enough_memory);
	else
		ret.reset(px, true);
	return ret;
}

latin1_prober::latin1_prober() noexcept:
	prober(),
	state_(prober::state_t::detecting),
	last_char_class_(OTH),
	freq_counter_({0})
{
}

uint16_t latin1_prober::get_charset_code() const noexcept
{
#ifndef __IO_WINDOWS_BACKEND__
	// Windows-1252 Latin 1
	return 1252;
#else
	// ISO_8859_1
	return 28591;
#endif // __IO_WINDOWS_BACKEND__
}

prober::state_t latin1_prober::handle_data(std::error_code& ec, const uint8_t* buff, std::size_t size) noexcept
{
	byte_buffer filtered = prober::filter_with_english_letters(ec,buff,size);
	if(!ec) {
		std::size_t char_class;
		std::size_t frequency;
		for(auto it = filtered.position(); it != filtered.last(); ++it ) {
			char_class = static_cast<std::size_t>( CHAR_TO_CLASS[ static_cast<std::size_t>(*it) ] );
			std::size_t idx = (last_char_class_ * CLASS_NUM) + char_class;
			frequency = static_cast<std::size_t>( CLASS_MODEL[idx] );
			if(0 == frequency) {
				state_ = prober::state_t::notme;
				break;
			}
			freq_counter_[ frequency] = freq_counter_[ frequency ] + 1;
			last_char_class_ = char_class;
		}
	}
	return state_;
}

prober::state_t latin1_prober::state() noexcept
{
	return state_;
}

void latin1_prober::reset() noexcept
{
	state_ = prober::state_t::detecting;
	last_char_class_ = OTH;
	std::fill(freq_counter_.begin(),freq_counter_.end(),0);
}

float latin1_prober::confidence() noexcept
{
	float ret;
	if(prober::state_t::notme == state_) {
		ret = 0.01f;
	}
	else {
		std::size_t total = std::accumulate(freq_counter_.begin(),freq_counter_.end(), 0 );
		if(0 == total) {
			ret =  0.0f;
		}
		else {
			ret = static_cast<float>(freq_counter_[3])  * (1.0f  / static_cast<float>(total));
			ret -= static_cast<float>(freq_counter_[1]) * (20.0f / static_cast<float>(total));
		}
	}
	return ret;
}

} // namespace detail

} // namespace io
