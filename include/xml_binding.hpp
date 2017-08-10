/*
 *
 * Copyright (c) 2016-2017
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

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include <algorithm>
#include <type_traits>

#include "tuple_meta_reflect.hpp"
#include "xml_types.hpp"
#include "xml_lexcast.hpp"
#include "xml_parse.hpp"


namespace io {

namespace xml {

namespace detail {

template<class xs_type, bool simple, bool attribute, bool tags_list>
class unmarshaller_functor
{
};

template<class xs_type>
class unmarshaller_functor<xs_type,true, true, false>
{
public:
	unmarshaller_functor(const s_event_stream_parser& parser) noexcept:
		parser_(parser)
	{}
	inline void operator()(xs_type& unmp,const start_element_event& e) const
	{
		static_assert(xs_type::attribute, "Assume attribute type only");
		const char* attr_name = unmp.name();
		for(auto it = e.attr_begin(); it != e.attr_end(); ++it) {
			if( it->name().eq( attr_name ) ) {
				//if( std::is_ )
				//unmp.set_value( lc_t::from_string( it->value().data() ) );
				// unmp.unmarshal( it->value().data() );
                return;
			}
		}
		std::string msg("Invalid XML structure, no ");
		msg.append(attr_name);
		msg.append(" attribute");
		throw std::runtime_error( msg );
	}
private:
	typedef typename xs_type::mapping_type attr_type;
	attr_type convert(const const_string& str) const {
		/*if( std::is_arithmetic<mapping_type>::value) {
			typedef typename lexical_cast_traits<attr_type, char8_lexical_cast> lc_t;
			return lc_t::from_stiring( str.data() );
		} else if( is_time_stamp<mapping_type>::value ) {

		}
		return std::string( str.data() );
		*/
	}

private:
	s_event_stream_parser parser_;
};

} // namespace detail

template <class xml_type>
class unmarshaller
{
public:

	inline void unmarshall(std::error_code& ec,const s_event_stream_parser& parser, xml_type& to) noexcept
	{
	}

};

} // namespace xml

} // namesapce io

#endif // __IO_XML_BINDING_HPP_INCLUDED__
