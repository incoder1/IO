
#include <text.hpp>
#include <fstream>
#include <iostream>

#include "stubs.hpp"

static const char* PROLOGUE = "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>";


#ifdef IO_XML_HAS_TO_XSD
static const char* SCHEMA_BEGIN = "<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\">";

void generate_xsd(app_settings& root) {
	std::fstream xsd("generated-app-config.xsd", std::ios_base::binary | std::ios_base::out);
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
	xsd.close();
}
#endif // IO_XML_HAS_TO_XSD


int main()
{

	app_settings root( primary_conf(1) );

	std::time_t cts = std::time(nullptr);
	std::tm *tm =  std::gmtime(&cts);

	root.add_conf( config(1, true,  std::move(*tm), "First configuration") );
	root.add_conf( config(2, false, std::move(*tm), "Second configuration") );

	app_settings::xml_type xt = root.to_xml_type();
	std::fstream file("app-config.xml", std::ios_base::binary | std::ios_base::out );
	file << io::unicode_cp::utf8;
	file << PROLOGUE;
	xt.marshal(file,0);

	std::cout<<"Resulting XML available in app-conf.xml:"<<std::endl;
	std::cout<<PROLOGUE<<std::endl;
	xt.marshal(std::cout, 1);

#if IO_XML_HAS_TO_XSD
	generate_xsd(root);
#endif // IO_XML_HAS_TO_XSD

	return 0;
}
