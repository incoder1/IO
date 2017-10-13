// This example program shows how to parse an XML using StAX API of
// IO library.
// Example usage: xmlparse < xmlfilename[.xml] >
// supported XML file text encoding Latin1(ASCII,ISO,Win1252 etc),
// UTF-8, UTF-16 (LE,BE) or UTF-32(LE,BE)
// for non UTF-8 encoding XML file should have byte order mark (BOM)

// mini embedded version of winver.h
// needed bu MinGW/64 headers wich uses WinXP by
// default
// We need Windows Vista +
#ifdef _WIN32
#	if _WIN32_WINNT < 0x0600
#		undef  _WIN32_WINNT
#		define WINVER 0x0600
#		define _WIN32_WINNT 0x0600
#	endif // _WIN32_WINNT
#endif

// for file channesl
#include <files.hpp>
// StAX XML parser
#include <xml_parse.hpp>
// Unicode console
#include <console.hpp>

using namespace io;

// print XML start document event e.g. XML prologure section details
void print_start_doc(std::ostream& stm,const xml::s_event_stream_parser& s)
{
	// extract and prase XML prologure
	xml::document_event e = s->parse_start_doc();
	// check parsing was succsess
	if( !s->is_error() ) {
		stm<< "start document:\n";
		stm<<"\tversion: " << e.version();
		stm<<" encoding: " << e.encoding();
		stm<<" standalone: " << (e.standalone() ? "yes" : "no") << std::endl;
	}
}

// print XML processing instruction details
static void print_instr(std::ostream& stm, const xml::s_event_stream_parser& s)
{
	// extract and parse processing instruction
	xml::instruction_event e = s->parse_processing_instruction();
	// check parsing was succsess
	if( s->is_error() ) {
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
	// check parsing was succsess
	if( !s->is_error() ) {
		stm<<"Start element:\n";
		stm<<"\tprefix:"<<e.name().prefix();
		stm<<" name:"<< e.name().local_name();
		// show whether  <tag attr="att"/> or <tag></tag>
		// parser not generating end element events for
		// self closing tags
		stm<<" empty element:" << ( e.empty_element() ? " yes" : " no") <<  '\n';
		// loop over the XML attributes if any
		if( e.has_attributes() ) {
			stm<<"\tattributes:\n";
			xml::start_element_event::iterator i = e.attr_begin();
			do {
				stm<<"\t\tname: " << i->name();
				stm<<" value: " << i->value() << '\n';
				++i;
			} while(i != e.attr_end() );
		}
		stm.flush();
	}
}

// print XML end element details
static void print_end_element(std::ostream& stm, const xml::s_event_stream_parser& s)
{
	xml::end_element_event e = s->parse_end_element();
	if(!s->is_error()) {
		stm<<"End element:\n\tprefix:" << e.name().prefix();
		stm<<" name:" << e.name().local_name() << std::endl;
	}
}

// choose a print operation according to the event type
static void print_event(std::ostream& stm,const xml::s_event_stream_parser& s)
{
	switch( s->current_event() ) {
	// this is start document event, i.e. XML prologure section
	case xml::event_type::start_document:
		print_start_doc(stm, s );
		break;
	// this is an XML processing istruction event
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
static  void log_chars(std::ostream& stm,const char* msg, const char* chars)
{
	stm<< msg << '\n' << chars << std::endl;
}

// handle unexpected error if any
// current implementation
// print last error (errno for UNIX or GetLastError for Windows) message into
// standard error stream and calls for std::exit with error number as a process
// execution result
static void on_terminate() noexcept
{
	io::exit_with_current_error();
}

// entry point to xml parsing
int main(int argc, const char** argv)
{
	// set terminate handler for unexpected errors if any
	std::set_terminate( on_terminate );
	// Take a console unicode out and error streams
	// can be replaced with std::cout and std::cerr
	// but no any guaranty for unicode support
	// (Windows CRT not supporting it for sure)
	std::ostream& cout = io::console::out_stream();
	std::ostream& cerr = io::console::error_stream();
	// take program arguments
	if(argc < 2) {
		cout<< "XML parsing example\n Usage:\t xmlparse <xmlfile>" <<std::endl;
		return 0;
	}
	// error code variable
	std::error_code ec;
	// obtain file descriptor
	file sf( argv[1] );
	if( !sf.exist() ) {
		cerr << sf.path() << " can not be found" << std::endl;
		return ec.value();
	}
	// Construct XML source
	xml::s_source src = xml::source::create(ec, sf.open_for_read(ec) );
	io::check_error_code( ec );
	// Construct StAX parser
	xml::s_event_stream_parser xs = xml::event_stream_parser::open(ec, src);
	io::check_error_code( ec );
	// loop for StAX parsing
	xml::state state;
	do {
		// Scan for next XML state
		state = xs->scan_next();
		switch(state.current) {
		// skip initial state and endof document state
		case xml::state_type::initial:
		case xml::state_type::eod:
			break;
		// next step is DTD section or link to DTD
		// you can read or skip it
		case xml::state_type::dtd:
			log_chars(cout,"DTD:", xs->read_dtd().data() );
			//xs->skip_dtd();
			break;
		// next state is an XML event, jump into event type swithc
		case xml::state_type::event:
			print_event(cout,xs);
			break;
		// next state is an XML comment, it is posible to read or to
		// skip it
		case xml::state_type::comment:
			log_chars(cout, "Comment: ", xs->read_comment().data() );
			// xs->skip_comment();
			break;
		// next step is <![CDATA section same as characters
		// but allow internal markup
		case xml::state_type::cdata:
			log_chars(cout,"CDATA: ", xs->read_cdata().data() );
			break;
		// next step is characters, i.e. tag value
		case xml::state_type::characters:
			log_chars(cout,"Characters: ", xs->read_chars().data() );
			break;
		}
	// until state is enf of document
	} while(xml::state_type::eod != state.current);
	// check whether there were an error
	if( xs->is_error() ) {
		// convert error enumeration into std::error_code
		// to get string error message
		xs->get_last_error(ec);
		// Obtain error location (line number and row) from XML source
		// and output error details into error stream
		cerr<< "XML error [" << xs->row() << ',' << xs->col() << "] " << ec.message() << std::endl;
		return -1;
	}
	// Parsing is done
	cout << "End of document" << std::endl;
	return 0;
}
