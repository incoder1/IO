/*
 *
 * Copyright (c) 2016
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#include "xml_error.hpp"

namespace io {

namespace xml {

const char* error_category::name() const noexcept
{
	return "XML parsing error";
}

std::error_condition error_category::default_error_condition(int ec) const noexcept
{
	return std::error_condition( static_cast<int>(ec), *error_category::instance() );
}

bool error_category::equivalent (const std::error_code& code, int condition) const noexcept
{
	return code.value() == condition;
}

const char* error_category::cstr_message(int err_code) const noexcept
{
	error ec = static_cast<error>(err_code);
	switch(ec) {
	case error::ok:
		return "No errors";
	case error::io_error:
		return "System input/output error";
	case error::illegal_chars:
		return "Illegal Unicode characters online";
	case error::out_of_memory:
		return "Not enough memory";
	case error::illegal_name:
		return "Tag or attribute name is illegal";
	case error::illegal_markup:
		return "XML node markup is incorrect";
	case error::illegal_prologue:
		return "Illegal XML prologue declaration";
	case error::illegal_cdata:
		return "Illegal <!CDATA[]]> section";
	case error::illegal_dtd:
		return "Illegal DTD declaration";
	case error::Illegal_commentary:
		return "Illegal commentary";
	case error::root_element_is_unbalanced:
		return "Root element is unbalanced";
	case error::invalid_state:
		return "Can not perform requested operation in current state";
	case error::parse_error:
		break;
	}
	return "General XML parsing error";
}

} //  namespace xml

} // namespace io
