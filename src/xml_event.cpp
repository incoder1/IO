/*
 *
 * Copyright (c) 2016-2020
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

// attribute
attribute::~attribute() noexcept
{}

//start_element_event
start_element_event::start_element_event() noexcept:
	name_(),
	attributes_(),
	empty_element_(false)
{
}

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
#ifndef IO_NO_EXCEPTIONS
	try {
		return attributes_.emplace( std::forward<attribute>( attr )  ).second;
	} catch(...) {
		return false;
	}
#else
	return attributes_.emplace( std::forward<attribute>( attr )  ).second;
#endif // IO_NO_EXCEPTIONS
}

std::pair<const_string, bool> start_element_event::get_attribute(const char* prefix, const char* name) const noexcept
{
	iterator ret = std::find_if(attributes_.cbegin(), attributes_.cend(),
				[prefix,name] (const attribute& attr) noexcept {
								return attr.name().equal(prefix, name);
				} );
	if( attributes_.cend() != ret)
		return std::make_pair( ret->value(), true );
	else
		return std::make_pair( const_string(), false );
}

} // namesapce xml

} // namespace io
