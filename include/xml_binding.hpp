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
#ifndef __IO_XML_BINDING_HPP_INCLUDED__
#define __IO_XML_BINDING_HPP_INCLUDED__

#include "config.hpp"
#include "xml_types.hpp"
#include "xml_lexcast.hpp"
#include "xml_parse.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

namespace io {

namespace xml {


template <class xml_type>
class unmarshaller
{
public:
	typedef typename xml_type::mapping_type binded_type;

	inline void unmarshall(std::error_code& ec,const event_stream_parser& parser, binded_type& to) noexcept
	{
		start_element_event e = parser->parse_start_element();
		if( parser->is_error() ) {
			parser->get_last_error(ec);
			return;
		}
		if( ! e.empty_element() ) {

		}

	}
};

} // namespace xml

} // namesapce io

#endif // __IO_XML_BINDING_HPP_INCLUDED__
