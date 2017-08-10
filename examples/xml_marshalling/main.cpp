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

void generate_xsd(app_settings& root, std::ostream& cout) {
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

	io::console::reset_out_color( io::text_color::white);
	cout<<"Resulting XSD available in generated-app-config.xsd:"<< std::endl;
	io::console::reset_out_color( io::text_color::light_blue);
	rootel.to_xsd(cout);
	cout << "<xs:element name=\"" << rootel.name() << "\" ";
	cout << "type=\"" << io::xml::extract_xsd_type_name<app_settings>() << "\"/>";
	cout << "</xs:schema>" << std::endl;
}
#endif // IO_XML_HAS_TO_XSD


int main()
{
	io::console::reset_err_color( io::text_color::light_red);
	std::ostream& cout = io::console::out_stream();

	app_settings root( primary_conf(1) );

	auto now = std::chrono::system_clock::now();

	root.add_conf( config(1, true,  std::move(now), "First configuration") );
	root.add_conf( config(2, false, std::move(now), "Second configuration") );

	app_settings::xml_type xt = root.to_xml_type();

	io::channel_ostream<char> xml( create_file_channel("app-config.xml"));
	//std::fstream xml("app-config.xml", std::ios_base::binary | std::ios_base::out );
	xml << io::unicode_cp::utf8;
	xml << PROLOGUE;
	xt.marshal(xml,0);
	xml.flush();

#if IO_XML_HAS_TO_XSD
	generate_xsd(root,cout);
#endif // IO_XML_HAS_TO_XSD

// unmarshal back from memory
	//io::xml::unmarshaller<app_settings::xml_type> unmarshaller;
	app_settings setings2 = app_settings::from_xml_type(xt);
	app_settings::xml_type xt2 = root.to_xml_type();

	io::console::reset_out_color( io::text_color::white);
	cout<<"Resulting XML available in app-conf.xml:"<<std::endl;
	io::console::reset_out_color( io::text_color::light_green);
	cout<<PROLOGUE<<std::endl;
	xt.marshal(cout, 1);
	cout.flush();


	return 0;
}
