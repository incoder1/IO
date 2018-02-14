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
#include "stdafx.hpp"
#include "xml_event.hpp"

namespace io {

namespace xml {

//attribute
attribute::attribute(cached_string&& name,const_string&& value) noexcept:
		name_( std::forward<cached_string>(name) ),
		value_( std::forward<const_string>(value) )
{}


//start_element_event
start_element_event::start_element_event() noexcept:
	name_(),
	attributes_(),
	empty_element_(false)
{}

start_element_event::start_element_event(start_element_event&& rhs) noexcept:
	name_( std::move(rhs.name_) ),
	attributes_( std::move(rhs.attributes_) ),
	empty_element_(rhs.empty_element_)
{}

start_element_event::start_element_event(qname&& name, bool empty_element) noexcept:
	name_(std::forward<qname>(name)),
	attributes_(),
	empty_element_(empty_element)
{}

bool start_element_event::add_attribute(attribute&& attr) noexcept
{
	return attributes_.emplace( std::forward<attribute>( attr ) ).second;
}


} // namesapce xml

} // namespace io
