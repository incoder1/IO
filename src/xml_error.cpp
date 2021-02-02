/*
 *
 * Copyright (c) 2016-2021
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#include "stdafx.hpp"
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


#define ERRMSG_DECL(ID,msg)\
static const char *ID = #msg
ERRMSG_DECL(IO_ERR,"System input/output error");
ERRMSG_DECL(OFM_ERR,"Not enough memory");
ERRMSG_DECL(ILG_ATTR_ERR,"Tag have several attributes with the same name");
ERRMSG_DECL(ILG_PRL_ERR, "Illegal XML prologue declaration");
ERRMSG_DECL(ILG_CHARS_ERR, "Illegal UNICODE character");
ERRMSG_DECL(ILG_NAME_ERR,"Tag or attribute name is illegal");
ERRMSG_DECL(ILG_MARKUP_ERR,"XML node markup is incorrect");
ERRMSG_DECL(ILG_DTD_ERR,"Illegal DTD declaration");
ERRMSG_DECL(ILG_COMENT_ERR,"Illegal commentary");
ERRMSG_DECL(ILG_CDATA_ERR,"Illegal <!CDATA[]]> section");
ERRMSG_DECL(UNBL_ERR,"Root element is unbalanced");
ERRMSG_DECL(STATE_ERR, "Can not perform requested operation in current state");
ERRMSG_DECL(OK_MSG, "No errors");
#undef ERRMSG_DECL

const char* error_category::cstr_message(int err_code) const noexcept
{
	switch( static_cast<error>(err_code) ) {
	case error::io_error:
		return IO_ERR;
	case error::out_of_memory:
		return OFM_ERR;
	case error::illegal_attribute:
		return ILG_ATTR_ERR;
	case error::illegal_prologue:
		return ILG_PRL_ERR;
	case error::illegal_chars:
		return ILG_CHARS_ERR;
	case error::illegal_name:
		return ILG_NAME_ERR;
	case error::illegal_markup:
		return ILG_MARKUP_ERR;
	case error::illegal_dtd:
		return ILG_DTD_ERR;
	case error::illegal_commentary:
		return ILG_COMENT_ERR;
	case error::illegal_cdata_section:
		return ILG_CDATA_ERR;
	case error::root_element_is_unbalanced:
		return UNBL_ERR;
	case error::invalid_state:
		return STATE_ERR;
	case error::ok:
		return OK_MSG;
	default:
		io_unreachable
		return OK_MSG;
	}
}

} //  namespace xml

} // namespace io
