/*
This example extract XML from below into structure like

struct configuration
{
	std::size_t id;
	bool enabled;
	std::string name;
};

where id and enabled fields stored as XML attributes and name stored as a tag

<?xml version="1.0" encoding="UTF-8"?>
<configurations>
	<configuration id="0" enabled="true">
		<name>Test configuration <![CDATA[<0>]]></name>
	</configuration>
	<configuration id="1" enabled="false">
		<name>Test configuration <![CDATA[<1>]]></name>
	</configuration>
	<configuration id="2" enabled="false">
		<name>Test configuration <![CDATA[<2>]]></name>
	</configuration>
</configurations>

*/

// Mini WinVer.h for Windows
#ifdef _WIN32
#	if _WIN32_WINNT < 0x0600
#		undef  _WIN32_WINNT
#		define WINVER 0x0600
#		define _WIN32_WINNT 0x0600
#	endif // _WIN32_WINNT
#endif


#include <iostream>
#include <vector>

#include <files.hpp>
#include <xml_reader.hpp>
#include <xml_lexcast.hpp>

/// A structure to parse into
struct configuration {
	std::size_t id;
	bool enabled;
	std::string name;
};

std::ostream& operator<<(std::ostream& s, const configuration& cnd)
{
	s << "configuration id: " << cnd.id << " enabled: " << (cnd.enabled ? "yes" : "no ")  << " name: " << cnd.name;
	return s;
}

// for converting characters into std::size_t
// can be replaced with boost lexical_cast
// or  std::stringstream etc
typedef io::xml::lexical_cast_traits<std::size_t> size_t_cast;
typedef io::xml::lexical_cast_traits<bool> bool_cast;

// De-serialize configuration structure from XML reader
static configuration read_config(io::unsafe<io::xml::reader>& rd)
{
	configuration ret;
	io::xml::start_element_event bev = rd.next_tag_begin();
	// read id from attribute
	io::const_string tmp = bev.get_attribute("","id").first;
	ret.id = size_t_cast::from_string( tmp.data() );
	tmp = bev.get_attribute("","enabled").first;
	ret.enabled = bool_cast::from_string( tmp.data() );
	// read name value from tag
	rd.next_tag_begin();
		ret.name = std::string( rd.next_characters().data() );
	rd.next_tag_end();
	// read </configuration>
	rd.next_tag_end();
	return ret;
}

int main(int argc, const char** argv)
{

	io::file sf("test-config.xml");
	if( !sf.exist() ) {
		std::cerr << sf.path() << " is not exit" << std::endl;
		return -1;
	}
	std::error_code ec;
	io::xml::s_event_stream_parser psr = io::xml::event_stream_parser::open(ec, sf.open_for_read(ec) );
	io::check_error_code( ec );

	std::cout<< "Configurations read from XML\n" << std::endl;

	io::unsafe<io::xml::reader> rd( std::move(psr) );
	// goto <configurations>
	io::xml::start_element_event start_el = rd.next_tag_begin();
	if( !start_el.name().equal("","configurations") ) {
		std::cerr << "Unexpected element: " << start_el.name().local_name() << std::endl;
		return -1;
	}

	std::vector<configuration> configurations;

	// De-serialize configurations
	rd.to_next_state();
	while( rd.is_tag_begin_next() ) {
		configurations.emplace_back( read_config(rd) );
		rd.to_next_state();
	}

	// Display results
	for(configuration cnf: configurations)
		std::cout << '\t' << cnf << std::endl;

	return 0;
}
