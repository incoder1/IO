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

} // namespace xml

} // namesapce io

#endif // __IO_XML_BINDING_HPP_INCLUDED__
