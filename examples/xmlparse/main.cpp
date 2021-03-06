// This example program shows how to parse an XML using StAX API of
// IO library.
// Example usage: xmlparse < xmlfilename[.xml] >
// supported XML file text encoding Latin1(ASCII,ISO,Win1252 etc),
// UTF-8, UTF-16 (LE,BE) or UTF-32(LE,BE)
// For non UTF-8 UNICODE encodings XML file should have byte order mark (BOM)

// mini embedded version of winver.h
// needed by MinGW/64 headers which uses WinXP by
// default
// We need Windows Vista +
#ifdef _WIN32
#	if _WIN32_WINNT < 0x0600
#		undef  _WIN32_WINNT
#		define WINVER 0x0600
#		define _WIN32_WINNT 0x0600
#	endif // _WIN32_WINNT
#endif

// for file channels
#include <files.hpp>
// StAX XML parser
#include <xml_parse.hpp>

// C++ standard library
#include <algorithm>
#include <type_traits>

// Unicode console, if not supported by runtime
#if defined(__IO_WINDOWS_BACKEND__) && defined(UNICODE)
#	include <console.hpp>
#	define NEED_UNICODE_CONSOLE 1
# else
#	include <iostream>
#endif


using namespace io;

// print XML start document event e.g. XML prologue section details
void print_start_doc(std::ostream& stm,const xml::s_event_stream_parser& s)
{
	// extract and parse XML prologue
	xml::document_event e = s->parse_start_doc();
	// check parsing was success
	if( !s->is_error() ) {
	    stm << "start document:\n";
	    stm << "\tversion: " << e.version();
	    stm << " encoding: " << e.encoding();
	    stm << " standalone: " << (e.standalone() ? "yes" : "no") << std::endl;
	}
}

// print XML processing instruction details
static void print_instr(std::ostream& stm, const xml::s_event_stream_parser& s)
{
	// extract and parse processing instruction
	xml::instruction_event e = s->parse_processing_instruction();
	// check parsing was success
	if( !s->is_error() ) {
		stm<<"processing instruction:\n";
		stm<<"\ttarget:" <<e.target();
		stm<<"\n\tdata: " << e.data() << std::endl;
	}
}

// print XML start element details
static void print_start_element(std::ostream& stm, const xml::s_event_stream_parser& s)
{
	// extract and parse XML tag start
	xml::start_element_event e = s->parse_start_element();
	// check parsing was success
	if( !s->is_error() ) {
		stm << "Start element:\n";

		if( e.name().has_prefix() )
			stm << "\tprefix:"<<e.name().prefix();

		stm << " name:"<< e.name().local_name();
		// show whether  <tag attr="att"/> or <tag></tag>
		// parser not generating end element events for
		// self closing tags
		stm <<" empty element:" << ( e.empty_element() ? " yes" : " no") <<  '\n';
		// loop over the XML attributes if any
		if( e.has_attributes() ) {
			stm<<"\tattributes:\n";
			std::for_each(e.attr_begin(), e.attr_end(), [&stm] (const xml::attribute& attr) {
				io::xml::qname attr_name = attr.name();
				stm << "\t\t";
				if( attr_name.has_prefix() )
					stm << "prefix: " << attr_name.prefix() << ' ';
				stm << "name: " << attr_name.local_name();
				stm << " value: " << attr.value() << '\n';
			} );
		}
		// flush to console
		stm.flush();
	}
}

// print XML end element details
static void print_end_element(std::ostream& stm, const xml::s_event_stream_parser& s)
{
	xml::end_element_event e = s->parse_end_element();
	if(!s->is_error()) {
		io::xml::qname el_name = e.name();

		stm << "End element:\n";
		if( el_name.has_prefix() )
			stm << " prefix:" << el_name.prefix();

		stm << " name:" << el_name.local_name() << std::endl;
	}
}

// choose a print operation according to the event type
static void print_event(std::ostream& stm,const xml::s_event_stream_parser& s)
{
	switch( s->current_event() ) {
	// this is start document event, i.e. XML prologue section
	case xml::event_type::start_document:
		print_start_doc(stm, s );
		break;
	// this is an XML processing instruction event
	case xml::event_type::processing_instruction:
		print_instr( stm, s );
		break;
	// this is start element event
	case xml::event_type::start_element:
		print_start_element( stm, s );
		break;
	// this is end element event
	case xml::event_type::end_element:
		print_end_element( stm, s );
		break;
	}
}

// output a string into a stream
static void log_chars(std::ostream& strm,const char* msg, const const_string& chars)
{
	strm << msg << '\n' << chars << '\n';
	if(chars.size() > 80)
		strm.flush();
}

/// print xml characters
static void print_xml_characters(std::ostream& strm,const xml::s_event_stream_parser& s)
{
	const_string chars = s->read_chars();
	// avoid login a between tag separators like spaces and line endings
	// parser not allowed to ignore such chars according to W3C standard
	if( !s->is_error() && !chars.blank() )
		log_chars(strm, "Characters: ", chars);
}

#ifdef IO_NO_EXCEPTIONS
// handle unexpected error if any
// current implementation
// print last error (errno for UNIX or GetLastError for Windows) message into
// standard error stream and calls for std::exit with error number as a process
// execution result
static void on_terminate() noexcept
{
	io::exit_with_current_error();
}
#endif // IO_NO_EXCEPTIONS

// entry point to xml parsing
int main(int argc, const char** argv)
{

#ifdef IO_NO_EXCEPTIONS
    // set terminate handler for unexpected errors if any
	std::set_terminate( on_terminate );
#endif // IO_NO_EXCEPTIONS

// Unicode console for Windows (not needed for MSYS2/Cygwin or UNIX)
#ifdef NEED_UNICODE_CONSOLE
	std::ostream& cout = io::console::out_stream();
	std::ostream& cerr = io::console::error_stream();
#else
	std::ostream& cout = std::cout;
	std::ostream& cerr = std::cerr;
#endif
	// take program arguments
	if(argc < 2) {
		cout<< "XML parsing example\n Usage:\t xmlparse <xmlfile>[.xml]" << std::endl;
		return 0;
	}
	// error code variable
	std::error_code ec;
	// obtain file descriptor
	file sf( argv[1] );
	if( !sf.exist() ) {
		cerr << sf.path() << " can not be found" << std::endl;
		return ec.value();
	} else {
		cout <<  " Parsing " << sf.path() << ' ' << sf.size() << " bytes" << std::endl;
	}
	// Construct XML source
	// Source will auto-detect XML file encoding, i.e. UTF-8, UTF-16[LE|BE] etc.
	xml::s_source src = xml::source::create(ec, sf.open_for_read(ec) );
	io::check_error_code( ec );
	// Construct StAX parser
	xml::s_event_stream_parser xs = xml::event_stream_parser::open(ec, std::move(src) );
	io::check_error_code( ec );
	// loop for StAX parsing
	xml::state_type state;
	do {
		// Scan for next XML state
		state = xs->scan_next();
		switch(state) {
		// skip initial state and end of document state
		case xml::state_type::initial:
		case xml::state_type::eod:
			break;
		// next step is DTD section or link to DTD
		// you can read or skip it
		case xml::state_type::dtd:
			log_chars(cout,"DTD:", xs->read_dtd() );
			//xs->skip_dtd();
			break;
		// next state is an XML event, jump into event type switch
		case xml::state_type::event:
			print_event(cout,xs);
			break;
		// next state is an XML comment, it is possible to read or to
		// skip it
		case xml::state_type::comment:
			log_chars(cout, "Comment: ", xs->read_comment() );
			// xs->skip_comment();
			break;
		// next step is <![CDATA section same as characters
		// but allow internal XML markup
		case xml::state_type::cdata:
			log_chars(cout, "CDATA: ", xs->read_cdata() );
			break;
		// next step is characters, i.e. tag value
		// optionally can be skip
		case xml::state_type::characters:
			print_xml_characters(cout, xs );
			// xs->skip_chars();
			break;
		}
	// until state is end of document
	} while(xml::state_type::eod != state);
	// check whether there were an error
	if( xs->is_error() ) {
		// convert error enumeration into std::error_code
		// to get string error message
		xs->get_last_error(ec);
		// Obtain error location (line number and row) from XML source
		// and output error details into error stream
		cerr<< "XML error [" << xs->row() << ',' << xs->col() << "] " << ec.message() << std::endl;
		// return error code
		return ec.value();
	}
	// Parsing is done
	if( xs->row() > 1 )
		cout << "End of document\n\t" <<  xs->row() << " rows processed" << std::endl;
	else
		cout << "End of document\n\t" <<  xs->col() << " symbols processed" << std::endl;
	return 0;
}
