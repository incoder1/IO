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
#include "xml_event_writer.hpp"

namespace io {

namespace xml {

static const char* PROLOGUE_FMT = "<?xml version=\"%s\" encoding=\"%s\"%s?>";
static const char* STANDALONE_ATTR = "standalone=\"yes\" ";


s_event_writer event_writer::open(std::error_code& ec, const s_write_channel& to, bool format, const document_event& prologue) noexcept
{
	char tmp[64] = {'\0'};
	const char* standalone = prologue.standalone() ? STANDALONE_ATTR : " ";
	std::snprintf( tmp, sizeof(tmp), PROLOGUE_FMT, prologue.version().data(), prologue.encoding().data(), standalone);
	if(format)
        tmp[ io_strlen(tmp) ] = '\n';
	writer dst( to );
	dst.write(ec, tmp);
	if(ec)
		return s_event_writer();
	io::byte_buffer buff = io::byte_buffer::allocate(ec, io::memory_traits::page_size() );
	if(ec)
		return s_event_writer();
	event_writer *ret = new (std::nothrow) event_writer(format, std::move(dst), std::move(buff) );
	return nullptr != ret ? s_event_writer(ret) : s_event_writer();
}

s_event_writer event_writer::open(std::error_code& ec,const s_write_channel& to, bool format, const version& v,const charset& encoding, bool standalone) noexcept {
	char vstr[8] = {'\0'};
	std::snprintf(vstr, sizeof(vstr), "%d.%d", v.major, v.minor);
	return open(ec, to, format, io::xml::document_event(vstr, encoding.name(), standalone) );
}

event_writer::event_writer(bool format, writer&& to, io::byte_buffer&& buff) noexcept:
	object(),
	nesting_level_( 0 ),
	format_( format ),
	to_( std::forward< writer >(to) ),
	buff_( std::forward< io::byte_buffer >(buff) )
{}

event_writer::~event_writer() noexcept
{
	if( !buff_.empty() ) {
		std::error_code ec;
		overflow(ec);
	}
}

void event_writer::overflow(std::error_code& ec) noexcept
{
	buff_.flip();
	to_.write( ec, buff_.position().cdata(), buff_.size() );
	buff_.clear();
}

void event_writer::print(std::error_code& ec,const char* str, std::size_t len) noexcept
{
	if(buff_.available() < len)
		overflow(ec);
	if(!ec)
		buff_.put(str, len);
}

void event_writer::print(std::error_code& ec,const char* str) noexcept
{
	print(ec, str, io_strlen(str) );
}

void event_writer::print(std::error_code& ec,const const_string& c) noexcept
{
	print(ec, c.data(), c.size() );
}

void event_writer::print(std::error_code& ec,const qname& name) noexcept
{
	if( name.has_prefix() ) {
		print(ec, name.prefix() );
		print(ec, ":", 1);
	}
	print(ec, name.local_name() );
}

void event_writer::independent(std::error_code& ec) noexcept
{
	for(std::size_t i=0; i < nesting_level_; i++) {
		print(ec, "\t");
	}
}

void event_writer::add(std::error_code& ec,const start_element_event& ev) noexcept
{
    // TODO:: state check's must present
  	if(format_)
    	independent(ec);
	print(ec, "<", 1);
	print(ec, ev.name() );
    if( ev.has_attributes() ) {
		for( auto it = ev.attr_begin(); !ec && it != ev.attr_end(); ++it ) {
			print(ec, " ");
			print(ec, it->name() );
			print(ec, "=\"");
			print(ec, it->value() );
			print(ec, "\"");
		}
    }
    if( ev.empty_element() ) {
    	print(ec, "/>");
    } else {
		print(ec,">");
		if(format_) {
			++nesting_level_;
			print(ec, "\n");
		}
    }
}

void event_writer::add(std::error_code& ec,const end_element_event& ev) noexcept
{
  	if(format_) {
		print(ec, "\n");
    	independent(ec);
  	}
	print(ec, "</");
	print(ec, ev.name() );
	print(ec, ">");
	if(format_)
		--nesting_level_;
}

void event_writer::add_cdata(std::error_code& ec,const char* str) noexcept
{
	print(ec, "<![CDATA[");
	print(ec, str);
	print(ec, "]]>");
}

void event_writer::add_chars(std::error_code& ec,const char* str) noexcept
{
	// TODO: characters validation
	// for CDATA
  	if(format_)
    	independent(ec);
	print(ec, str);
}

void event_writer::add_coment(std::error_code& ec,const char* str) noexcept
{
	// TODO: validate for double -- inside the comment
  	if(format_)
    	independent(ec);
    print(ec, "<!--");
    print(ec, str);
    print(ec, "-->");
    if(format_)
    	print(ec, "\n");
}

} // namespace xml

} // namespace io
