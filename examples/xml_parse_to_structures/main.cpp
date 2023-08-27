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

#include <iostream>
#include <vector>

#include <files.hpp>
#include <xml/reader.hpp>
#include <char_cast.hpp>

/// A structure to parse into
struct configuration {
	std::size_t id;
	bool enabled;
	std::string name;
	configuration();
};

configuration::configuration():
	id(0),
	enabled(false),
	name()
{}

std::ostream& operator<<(std::ostream& s, const configuration& cnd)
{
	s << "configuration id: " << cnd.id << " enabled: " <<  io::to_string(cnd.enabled, io::str_bool_format::yes_no)  << " name: " << cnd.name;
	return s;
}

// Get required XML tag attribute string value or error when no such attribute
static io::const_string get_string_attr(const io::xml::start_element_event& ev,const char* name)
{
	io::const_string ret;
#ifdef __HAS_CPP_17
	std::optional<io::const_string> attr = ev.get_attr("",name);
	if(attr)
		ret = *attr;
	else
		io::check_error_code( io::xml::make_error_code(io::xml::error::illegal_attribute) );
#else
	auto attr = ev.get_attribute("",name);
	if(attr.second)
		ret = attr.first;
	else
		io::check_error_code( io::xml::make_error_code(io::xml::error::illegal_attribute) );
#endif // __HAS_CPP_17
	return ret;
}

// Get required size_t XML attribute
static std::size_t get_sizet_attr(const io::xml::start_element_event& ev,const char* name)
{
	std::size_t ret;
	io::from_string(get_string_attr(ev,name), ret);
	return ret;
}

// Get required boolean attribute
static bool get_bool_attr(const io::xml::start_element_event& ev,const char* name)
{
	bool ret;
	io::from_string(get_string_attr(ev,name), ret);
	return ret;
}

// De-serialize configuration structure from XML reader
static configuration read_config(io::unsafe<io::xml::reader>& rd)
{
	configuration ret;
	// taking next start element event and check whether it <configuration>
	io::xml::start_element_event sev = rd.next_expected_tag_begin("configuration");
	// obtain id="123" attribute value
	ret.id = get_sizet_attr(sev,"id");

	// obtain enabled="true|false" attribute value
	ret.enabled = get_bool_attr(sev,"enabled");

	// read nesting <name>name</name> tag value
	rd.next_expected_tag_begin("name");
	ret.name = rd.next_characters().clone();
	rd.next_expected_tag_end("name");

	// throw to next </configuration> tag end or error if it is not
	rd.next_expected_tag_end("configuration");
	return ret;
}

int main(int argc, const char** argv)
{
	std::ios::sync_with_stdio(false);
	io::file sf("test-config.xml");
	if( !sf.exist() ) {
		std::cerr << sf.path() << " is not exist" << std::endl;
		return -1;
	}

	std::error_code ec;

	// Open a XML parser from file channel
	io::xml::s_event_stream_parser psr = io::xml::event_stream_parser::open(ec, sf.open_for_read(ec) );
	io::check_error_code( ec );

	std::cout<< "Configurations read from XML\n" << std::endl;

	// Construct XML reader, unsafe throws exceptions on XML parsing error
	// when compiler currently supports exceptions,
	// or print error into error stream and stop process with std::exit
	// when exceptions has been off by compiler options
	io::unsafe<io::xml::reader> rd( std::move(psr) );

	// goto <configurations>
	rd.next_expected_tag_begin("configurations");

	std::vector<configuration> configurations;

	// De-serialize configurations
	rd.to_next_state();
	while( rd.is_tag_begin_next() ) {
		configurations.emplace_back( read_config(rd) );
		rd.to_next_state();
	}

	// check end element event
	rd.next_expected_tag_end("configurations");

	// Display results
	for(configuration cnf: configurations) {
		std::cout << '\t' << cnf << std::endl;
	}

	return 0;
}
