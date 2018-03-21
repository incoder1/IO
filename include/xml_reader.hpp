/*
 *
 * Copyright (c) 2016-2018
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef __IO_XML_READER_HPP_INCLUDED__
#define __IO_XML_READER_HPP_INCLUDED__

#include "config.hpp"

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

#include "errorcheck.hpp"
#include "xml_parse.hpp"

namespace io {

namespace xml {

/// XML Parser cursor API
class IO_PUBLIC_SYMBOL reader {
public:
	/// Construct an XML reader on top of StAX parser
	/// \param parser StAX parser instance
	explicit reader(s_event_stream_parser&& parser) noexcept;
	/// Drop parser to the next tag
	/// Check for tag self closing by start_element_event#empty_element
	/// \param ec operation error code
	/// \return start element event
	start_element_event next_tag_begin(std::error_code& ec) noexcept;
	/// Drop parser to the next tag end
	/// WARN. Self closed tags do not handled by this method
	/// \param ec operation error code
	/// \return end element event
	end_element_event next_tag_end(std::error_code& ec) noexcept;
	/// Read current characters
	/// \param ec operation error code
	/// \return tag characters
	const_string next_characters(std::error_code& ec) noexcept;

private:
	inline bool parse_error(std::error_code& ec) noexcept {
		if(ec)
			return true;
		if( parser_->is_error() ) {
			parser_->get_last_error(ec);
			return true;
		}
		return false;
	}

	inline bool is_characters() noexcept {
		return state_type::characters == state_ || state_type::cdata == state_;
	}

	void to_next_state(std::error_code& ec) noexcept;
private:
	s_event_stream_parser parser_;
	state_type state_;
};


} // namespace xml

template<>
class unsafe<xml::reader>
{
public:
	unsafe(xml::reader&& rd) noexcept:
		rd_(std::forward<xml::reader>(rd)),
		ec_()
	{}
	unsafe(xml::s_event_stream_parser&& parser) noexcept:
		rd_(std::forward<xml::s_event_stream_parser>(parser)),
		ec_()
	{}
	xml::start_element_event next_tag_begin() noexcept
	{
		xml::start_element_event ret = rd_.next_tag_begin(ec_);
		check_error_code(ec_);
		return std::move(ret);
	}
	xml::end_element_event next_tag_end() noexcept
	{
		xml::end_element_event ret = rd_.next_tag_end(ec_);
		check_error_code(ec_);
		return std::move(ret);
	}
	const_string next_characters() noexcept
	{
		const_string ret = rd_.next_characters(ec_);
		check_error_code(ec_);
		return std::move(ret);
	}
private:
	xml::reader rd_;
	std::error_code ec_;
};

} // namespace io

#endif // __IO_XML_READER_HPP_INCLUDED__
