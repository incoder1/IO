/**
 * This example shows ho to write any kind of XML using IO streaming event API
 */
// IO library
#include <io/core/console.hpp>
#include <io/core/char_cast.hpp>
#include <io/core/files.hpp>
#include <io/xml/event_writer.hpp>

namespace xml = io::xml;

int main(int argc, const char** argv)
{

	// We will serialize our XML into console
	io::console cons;
	//io::console_out_writer out(cons);

	std::error_code ec;
	io::file dst_file("out.xml");

	io::s_write_channel out = dst_file.open_for_write(ec, io::write_open_mode::overwrite);
	io::check_error_code(ec);

	//out->write(ec, utf_16le_bom::data(), utf_16le_bom::len() );
	//io::check_error_code(ec);

	//io::s_charset_converter cvt = io::charset_co

	io::s_funnel fnl = io::buffered_channel_funnel::create(ec, std::move(out), 32 );
	io::check_error_code(ec);
	io::writer fout(fnl);

	// Open the event streaming event writer

	xml::s_event_writer safe_ew = xml::event_writer::open( ec, std::move(fout), true, {1,0}, io::code_pages::utf8(), false);
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
