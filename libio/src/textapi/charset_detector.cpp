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

#include <io/core/string_algs.hpp>

#include "io/textapi/charset_detector.hpp"

#include "io/textapi/detail/latin1_prober.hpp"
#include "io/textapi/detail/utf8_prober.hpp"

/*
 This is C++ 11 minimal port of Mozilla universal character set detector
 FIXME: Those state machines is over-complicated, an performance un-effective
  Needed to be replaced with something else like https://github.com/google/compact_enc_det
*/
namespace io {

//charset_detector
s_charset_detector charset_detector::create(std::error_code& ec) noexcept
{
	s_charset_detector ret;
	if(!ec) {
		detail::s_prober latin1_prb = detail::latin1_prober::create(ec);
		detail::s_prober utf8_prb = detail::utf8_prober::create(ec);
		if(!ec) {
			std::array<detail::s_prober, 2> probers = {latin1_prb,utf8_prb};
			charset_detector *px = new (std::nothrow) charset_detector(std::move(probers));
			if(nullptr == px)
				ec = std::make_error_code(std::errc::not_enough_memory);
			else
				ret.reset(px, true);
		}
	}
	return ret;
}

charset_detector::charset_detector(v_pobers&& probers) noexcept:
	object(),
	probers_( std::forward<v_pobers>(probers) )
{}

charset_detect_status charset_detector::detect(std::error_code &ec,const uint8_t* buff, std::size_t size) const noexcept
{
	// try unicode byte order mark first
	unicode_cp unicp = detect_by_bom(buff);
	switch(unicp) {
	case unicode_cp::not_detected:
		break;
	case unicode_cp::utf8:
		return charset_detect_status(code_pages::utf8(), 1.0f);
	case unicode_cp::utf_16be:
		return charset_detect_status(code_pages::utf16be(), 1.0f);
	case unicode_cp::utf_16le:
		return charset_detect_status(code_pages::utf16le(), 1.0f);
	case unicode_cp::utf_32be:
		return charset_detect_status(code_pages::utf32be(), 1.0f);
	case unicode_cp::utf_32le:
		return charset_detect_status(code_pages::utf32le(), 1.0f);
	}
	// FIXME insert implementation
	return charset_detect_status(nullptr, false);
}

} // namespace io
