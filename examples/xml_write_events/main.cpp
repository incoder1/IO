/**
 * This example shows ho to write any kind of XML using IO streaming event API
 */
// IO library
#include <console.hpp>
#include <char_cast.hpp>
#include <files.hpp>
#include <xml/event_writer.hpp>

namespace xml = io::xml;

int main(int argc, const char** argv)
{

	// We will serialize our XML into console
	io::console cons;
	io::console_out_writer out(cons);

	// Open the event streaming event writer
	std::error_code ec;
	xml::s_event_writer safe_ew = xml::event_writer::open( ec, std::move(out), true, {1,0}, io::code_pages::UTF_8, false);
	io::check_error_code(ec);
	// make unsafe concept, to avoid manual error checking, i.e. use exceptions if they are on
	io::unsafe<xml::event_writer> xew( std::move(safe_ew) );

	xew.add_coment("Test event writer");
	xml::start_element_event sev( xml::qname("tst","test"), false );

	// crate a root element and provide namespaces
	sev.add_attribute(xml::attribute( xml::qname("xmlns","xsi"), "http://www.w3.org/2001/XMLSchema-instance") );
	sev.add_attribute(xml::attribute( xml::qname("xmlns","tst"), "https://github.com/incoder1/IO") );

	xew.add(sev);
	xew.add(xml::start_element_event(xml::qname("tst","cdata_chars"), false));
	xew.add_cdata("<Test CDATA >");
	xew.add(io::xml::end_element_event(xml::qname("tst","cdata_chars")));
	xew.add_chars("Test characters");

	xew.add(xml::end_element_event(xml::qname("tst","test")));

	return 0;
}
