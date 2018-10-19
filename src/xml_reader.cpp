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
#include "stdafx.hpp"
#include "xml_reader.hpp"

namespace io {

namespace xml {

reader::reader(s_event_stream_parser&& parser) noexcept:
	parser_( std::forward<s_event_stream_parser>(parser) ),
	state_()
{}

void reader::to_next_state(std::error_code& ec) noexcept
{
	bool done = false;
	do {
		state_ = parser_->scan_next();
		switch(state_) {
		case state_type::initial:
			break;
		case state_type::eod:
			ec = std::make_error_code(error::invalid_state);
			break;
		case state_type::comment:
			parser_->skip_comment();
			break;
		case state_type::dtd:
			parser_->skip_dtd();
			break;
		case state_type::characters:
			if( !parser_->read_chars().blank() ) {
				// error non blank characters between tags
				ec = std::make_error_code(error::invalid_state);
			}
			break;
		default:
			done = true;
			break;
		}
	}
	while( !parse_error(ec) && !done );
}

start_element_event reader::next_tag_begin(std::error_code& ec) noexcept
{
	if(state_type::event != state_)
		to_next_state(ec);
	bool scaning = true;
	while( scaning && !parse_error(ec) ) {
		switch( parser_->current_event() ) {
		case event_type::start_element:
			scaning = false;
			break;
		case event_type::end_element:
			ec = make_error_code(error::invalid_state);
			scaning = false;
			break;
		case event_type::start_document:
			parser_->parse_start_doc();
			to_next_state(ec);
			break;
		case event_type::processing_instruction:
			parser_->parse_processing_instruction();
			to_next_state(ec);
			break;
		}
	}
	if(!ec) {
		start_element_event ret = parser_->parse_start_element();
		if( parser_->is_error() )
			parser_->get_last_error(ec);
		state_ = parser_->scan_next();
		return std::move(ret);
	}
	return start_element_event();
}

bool reader::is_tag_begin_next() noexcept
{
	return state_type::event == state_ && event_type::start_element == parser_->current_event();
}

bool reader::is_characters_next() noexcept
{
	return state_type::characters == state_;
}

bool reader::is_tag_end_next() noexcept
{
	return state_type::event == state_ && event_type::end_element == parser_->current_event();
}

end_element_event reader::next_tag_end(std::error_code& ec) noexcept
{
	if(state_type::event != state_)
		to_next_state(ec);
	if(event_type::end_element != parser_->current_event()) {
		ec = make_error_code(error::invalid_state);
		return end_element_event();
	}
	end_element_event ret = parser_->parse_end_element();
	if( parser_->is_error() )
		parser_->get_last_error(ec);
	else
		state_ = parser_->scan_next();
	return std::move(ret);
}


const_string reader::next_characters(std::error_code& ec) noexcept
{
	if(state_type::characters != state_ && state_type::cdata != state_) {
		ec = make_error_code(error::invalid_state);
		return const_string();
	}
	byte_buffer buff = byte_buffer::allocate(ec, 128);
	if(ec)
		return const_string();
	std::size_t s;
	const_string chars;
	for(;;) {
		switch( state_ ) {
		case state_type::characters:
			chars = parser_->read_chars();
			break;
		case state_type::cdata:
			chars = parser_->read_cdata();
			break;
		case state_type::comment:
			parser_->skip_comment();
			break;
		case state_type::event:
			if( io_likely( event_type::processing_instruction != parser_->current_event() ) ) {
				buff.flip();
				return const_string(buff.position().cdata(), buff.length());
			}
			else
				parser_->parse_processing_instruction();
			break;
		case state_type::eod:
			parser_->get_last_error(ec);
			return const_string();
		case state_type::initial:
		case state_type::dtd:
			io_unreachable
			break;
		}
		state_ = parser_->scan_next();
		s = chars.size();
		if( io_unlikely(buff.available() < s && !buff.extend( s + 1 ) ) ) {
			ec = std::make_error_code( std::errc::not_enough_memory );
			return const_string();
		}
		buff.put( chars.data() );
	}
	io_unreachable
	return const_string();
}

} // namespace xml

} // namespace io
