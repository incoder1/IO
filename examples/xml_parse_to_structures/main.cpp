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
	<configuration id="0" enabled="yes">
		<name>Test configuration <![CDATA[<0>]]></name>
	</configuration>
	<configuration id="1" enabled="no">
		<name>Test configuration <![CDATA[<1>]]></name>
	</configuration>
	<configuration id="2" enabled="no">
		<name>Test configuration <![CDATA[<2>]]></name>
	</configuration>
</configurations>

*/

#include <vector>

#include <io/core/files.hpp>
#include <io/core/char_cast.hpp>
#include <io/core/console.hpp>

#include <io/xml/reader.hpp>

/// A structure to parse into
struct configuration {
	std::size_t id;
	bool enabled;
	std::string name;
};


/// XML syntax color highlight for tag
void format_tag(std::ostream& s, int nl, const char* name, bool close, bool hasattr)
{
	for(int i=0; i < nl; i++)
		s << "    ";
	if(close)
		s << "</";
	else
		s << '<';
	s << io::cclr::navy_green;
	s << name;
	if(!hasattr)
		s << io::cclr::reset << '>';
}

/// XML syntax color highlight for tag attribute
void format_attribute(std::ostream& s, const char* name, const io::const_string& val)
{
	s << ' ';
	s << io::cclr::brown;
	s << name;
	s << io::cclr::reset << "=\"";
	s << io::cclr::light_aqua << val;
	s << io::cclr::reset << '"';
}

/// XML syntax format a configuration with color highlight
void format_configuration(std::ostream& s, const configuration& cnd)
{
	format_tag(s, 1, "configuration",false,true);
	format_attribute(s, "id", io::to_string(cnd.id) );
	format_attribute(s, "enabled", io::to_string(cnd.enabled, io::str_bool_format::yes_no) );
	s << '>';
	s << std::endl;
	format_tag(s, 2, "name",false,false);
	s << io::cclr::yellow << "<![CDATA["<< io::cclr::reset;
	s << cnd.name;
	s << io::cclr::yellow << "]]>" << io::cclr::reset;;
	format_tag(s, 0, "name",true,false);
	s << std::endl;
	format_tag(s, 1, "configuration", true, false);
	s << std::endl;
}

/// Get required XML tag attribute string value or error when no such attribute
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
	io::console cons;
	io::console_output_stream cout(cons);
	io::console_error_stream cerr(cons);

	io::file sf("test-config.xml");
	if( !sf.exist() ) {
		cerr << sf.path() << " is not exist" << std::endl;
		return -1;
	}

	cout << "About to read and parse " << sf.path() << " file" << std::endl;

	std::error_code ec;

	// Open a XML parser from file channel
	io::xml::s_event_stream_parser psr = io::xml::event_stream_parser::open(ec, sf.open_for_read(ec) );
	io::check_error_code( ec );

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
	cout<< "Format structures back to XML into stream with color syntax highlighting\n" << std::endl;
	cout << io::cclr::navy_red << "<?" << io::cclr::reset << "xml ";
	format_attribute(cout, "version", "1.0");
	format_attribute(cout, "encoding", "UTF-8");
	cout << io::cclr::navy_red << "?>" << io::cclr::reset << std::endl;

	format_tag(cout,0,"configurations",false,false);
	cout << std::endl;
	for(auto it: configurations) {
		format_configuration(cout, it);
	}
	format_tag(cout,0,"configurations",true,false);
	return 0;
}
