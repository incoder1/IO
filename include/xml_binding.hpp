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
#include "tuple_meta_reflect.hpp"

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

	inline void unmarshall(std::error_code& ec,const s_event_stream_parser& parser, xml_type& to) noexcept
	{
		xml::state state;
		bool done = false;
		do {
			state = parser->scan_next();
			switch(state.current) {
				case xml::state_type::start_document:
					parser->parse
					continue;
				case xml::state_type::eod:
					done = true;
					break;
				case xml::state_type::characters:
					parser->
					break;
				case xml::state_type::dtd:

				case xml::state_type::comment:
					parser->skip_comment();
			}
		} while(!done);
	}

};

} // namespace xml

} // namesapce io

#endif // __IO_XML_BINDING_HPP_INCLUDED__
