#ifdef _WIN32
#define _WIN32_WINNT 0x0600
#endif // _WIN32

#include <console.hpp>
#include <files.hpp>
#include <stream.hpp>

#include "stubs.hpp"

static const char* PROLOGUE = "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>";


static io::s_write_channel create_file_channel(const char* path) {
	std::error_code ec;
	io::file f = io::file::get(ec,  path );
	io::check_error_code(ec);
	io::s_write_channel ret = f.open_for_write(ec, io::write_open_mode::overwrite);
	io::check_error_code(ec);
	return ret;
}

#ifdef IO_XML_HAS_TO_XSD
static const char* SCHEMA_BEGIN = "<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\">";

void generate_xsd(app_settings& root) {
	io::channel_ostream<char> xsd( create_file_channel("generated-app-config.xsd") );
	xsd << io::unicode_cp::utf8;
	xsd << PROLOGUE;
	xsd << SCHEMA_BEGIN;
	root.primary().to_xml_type().to_xsd(xsd);
	root.first_config()->to_xml_type().to_xsd(xsd);

	app_settings::xml_type rootel = root.to_xml_type();
	rootel.to_xsd(xsd);

	xsd << "<xs:element name=\"" << rootel.name() << "\" ";
	xsd << "type=\"" << io::xml::extract_xsd_type_name<app_settings>() << "\"/>";
	xsd << "</xs:schema>";
	xsd.flush();
}
#endif // IO_XML_HAS_TO_XSD


int main()
{
	io::console::reset_colors( io::text_color::yellow, io::text_color::light_green,  io::text_color::light_red );
	std::ostream& cout = io::console::out_stream();

	app_settings root( primary_conf(1) );

	std::time_t cts = std::time(nullptr);
	std::tm *tm =  std::gmtime(&cts);

	root.add_conf( config(1, true,  std::move(*tm), "First configuration") );
	root.add_conf( config(2, false, std::move(*tm), "Second configuration") );

	app_settings::xml_type xt = root.to_xml_type();

	io::channel_ostream<char> xml( create_file_channel("app-config.xml"));
	//std::fstream xml("app-config.xml", std::ios_base::binary | std::ios_base::out );
	xml << io::unicode_cp::utf8;
	xml << PROLOGUE;
	xt.marshal(xml,0);
	xml.flush();

	cout<<"Resulting XML available in app-conf.xml:"<<std::endl;
	cout<<PROLOGUE<<std::endl;
	xt.marshal(cout, 1);
	cout.flush();

#if IO_XML_HAS_TO_XSD
	generate_xsd(root);
#endif // IO_XML_HAS_TO_XSD

	return 0;
}
