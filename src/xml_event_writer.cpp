#include "stdafx.hpp"
#include "xml_event_writer.hpp"

namespace io {

namespace xml {

static const char* PROLOGUE_FMT = "<?xml version=\"%s\" encoding=\"%s\"%s?>";
static const char* STANDALONE_ATTR = "standalone=\"yes\" ";


s_event_writer event_writer::open(std::error_code& ec, const s_write_channel& to, const document_event& prologue) noexcept
{
	char tmp[64] = {'\0'};
	const char* standalone = prologue.standalone() ? STANDALONE_ATTR : " ";
	std::snprintf( tmp, sizeof(tmp), PROLOGUE_FMT, prologue.version().data(), prologue.encoding().data(), standalone);
	writer<char> dst( to );
	dst.write(ec, tmp);
	if(ec)
		return s_event_writer();
	event_writer *ret = new (std::nothrow) event_writer( std::move(dst) );
	return nullptr != ret ? s_event_writer(ret) : s_event_writer();
}

s_event_writer event_writer::open(std::error_code& ec,const s_write_channel& to,  const version& v,const charset& encoding, bool standalone) noexcept {
	char vstr[8] = {'\0'};
	std::snprintf(vstr, sizeof(vstr), "%d.%d", v.major, v.minor);
	return open(ec, to, io::xml::document_event(vstr, encoding.name(), standalone) );
}

event_writer::event_writer(writer<char>&& to) noexcept:
	object(),
	to_( std::forward< writer<char> >(to) )
{}

event_writer::~event_writer() noexcept
{}

void event_writer::print(std::error_code& ec,const char* str, std::size_t len) noexcept
{
	if(!ec)
		to_.write( ec, str, len );
}

void event_writer::print(std::error_code& ec,const char* str) noexcept
{
	if(!ec)
		to_.write(ec, str);
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

void event_writer::add(std::error_code& ec,const start_element_event& ev) noexcept
{
    // TODO:: state check's must present
	print(ec, "<", 1);
	print(ec, ev.name() );
    if( ev.has_attributes() ) {
		for( auto it = ev.attr_begin(); !ec && it != ev.attr_end(); ++it ) {
			print( ec, " ");
			print( ec, it->name() );
			print(ec, "=\"");
			print(ec, it->value() );
			print(ec, "\"");
		}
    }
    if( ev.empty_element() )
    	print(ec, "/>");
    else
		print(ec,">");
}

void event_writer::add(std::error_code& ec,const end_element_event& ev) noexcept
{
	print(ec, "</");
	print(ec, ev.name() );
	print(ec, ">");
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
	print(ec, str);
}

void event_writer::add_coment(std::error_code& ec,const char* str) noexcept
{
	// TODO: validate for double -- inside the comment
    print(ec, "<!--");
    print(ec, str);
    print(ec, "-->");
}

} // namespace xml

} // namespace io
