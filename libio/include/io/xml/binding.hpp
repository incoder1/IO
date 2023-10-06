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
#ifndef __IO_XML_BINDING_HPP_INCLUDED__
#define __IO_XML_BINDING_HPP_INCLUDED__

#include <io/config/libio_config.hpp>

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

template<class _tuple_t>
struct marshaller {
	static inline void marshal(const _tuple_t& t,std::error_code& ec, const xml::s_event_writer& to)
	{
		meta::for_each_in_tuple(  const_cast<_tuple_t&&>(t), [ec,to] { t.marshal(ec, to); } );
	}

	static inline void marshal(_tuple_t&& t,const xml::s_event_writer& t)
	{
		meta::for_each_in_tuple( std::forward<_tuple_t>(t), [ec,to] { t.marshal(ec, to); } );
	}

};

} // namespace xml

} // namesapce io

#endif // __IO_XML_BINDING_HPP_INCLUDED__
