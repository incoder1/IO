/*
 *
 * Copyright (c) 2016-2022
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
static const char* STANDALONE_ATTR = " standalone=\"yes\" ";

#ifdef __IO_WINDOWS_BACKEND__
static const char* WIDEN_ENDL = "\r\n";
#else
static const char* WIDEN_ENDL = "\n";
#endif // __IO_WINDOWS_BACKEND__


s_event_writer event_writer::open(std::error_code& ec, writer&& dst, bool format, const document_event& prologue) noexcept
{
	char tmp[64] = {'\0'};
	const char* standalone = prologue.standalone() ? STANDALONE_ATTR : " ";
	std::snprintf( tmp, sizeof(tmp), PROLOGUE_FMT, prologue.version().data(), prologue.encoding().data(), standalone);
	if(format)
		tmp[ io_strlen(tmp) ] = '\n';
	if(!dst) {
		ec = dst.last_error();
		return s_event_writer();
	}
	dst.write(tmp);
	event_writer *ret = new (std::nothrow) event_writer(format, std::forward<writer&&>(dst) );
	if(nullptr == ret) {
		ec = std::make_error_code(std::errc::not_enough_memory);
		return s_event_writer();
	}
	return s_event_writer(ret);
}

s_event_writer event_writer::open(std::error_code& ec,writer&& dst, bool format, const version& v,const charset& encoding, bool standalone) noexcept
{
	char vstr[8] = {'\0'};
	std::snprintf(vstr, sizeof(vstr), "%d.%d", v.major, v.minor);
	return open(ec, std::forward<writer&&>(dst), format, io::xml::document_event(vstr, encoding.name(), standalone) );
}

s_event_writer event_writer::open(std::error_code& ec, writer&& dst) noexcept
{
	return open(ec, std::forward<writer&&>(dst), true, {1,0}, code_pages::UTF_8, false );
}

event_writer::event_writer(bool format, writer&& to) noexcept:
	object(),
	nesting_level_( 0 ),
	format_( format ),
	to_( std::forward< writer >(to) ),
	ec_()
{}

event_writer::~event_writer() noexcept
{}


void event_writer::print(const char ch) noexcept
{
	if(!ec_) {
		to_.write(ch);
		if(!to_)
			ec_ = to_.last_error();
	}
}

void event_writer::print(const char* str) noexcept
{
	if(!ec_) {
		to_.write(str);
		if(!to_)
			ec_ = to_.last_error();
	}
}

void event_writer::print(const const_string& str) noexcept
{
	if(!ec_) {
		to_.write( str.data(), str.size() );
		if(!to_)
			ec_ = to_.last_error();
	}
}

void event_writer::print(const qname& name) noexcept
{
	if(!ec_) {
		if( name.has_prefix() ) {
			print(name.prefix());
			print(':');
		}
		print(name.local_name());
	}
}

void event_writer::independent() noexcept
{
	for(std::size_t i=0; to_ && i < nesting_level_; i++) {
		print('\t');
	}
	if(!to_)
		ec_ = to_.last_error();
}

void event_writer::add(const start_element_event& ev) noexcept
{
	// TODO: state check's must present
	if(format_)
		independent();
	print('<');
	print(ev.name());
	if( !ec_ && ev.has_attributes() ) {
		for( auto it = ev.attr_begin(); !ec_ && it != ev.attr_end(); ++it ) {
			print(' ');
			print(it->name());
			print("=\"");
			print(it->value());
			print('"');
		}
	}
	if( ev.empty_element() ) {
		print("/>");
	}
	else {
		print('>');
		if(format_) {
			++nesting_level_;
			print(WIDEN_ENDL);
		}
	}
}

void event_writer::add(const end_element_event& ev) noexcept
{
	if(format_) {
		--nesting_level_;
		print(WIDEN_ENDL);
		independent();
	}
	print("</");
	print(ev.name());
	print('>');
}

void event_writer::add_cdata(const char* str) noexcept
{
	if(format_)
		independent();
	print("<![CDATA[");
	print(str);
	print("]]>");
}

void event_writer::add_chars(const char* str) noexcept
{
	// TODO: characters validation
	// for CDATA
	if(format_) {
		print(WIDEN_ENDL);
		independent();
	}
	print(str);
}

void event_writer::add_coment(const char* str) noexcept
{
	// TODO: validate for double -- inside the comment
	if(format_)
		independent();
	print("<!--");
	print(str);
	print("-->");
	if(format_)
		print(WIDEN_ENDL);
}

} // namespace xml

} // namespace io
