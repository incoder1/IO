#if _WIN32_WINNT < 0x0600
#	undef  _WIN32_WINNT
#	define WINVER 0x0600
#	define _WIN32_WINNT 0x0600
#endif // _WIN32_WINNT

#include <files.hpp>
#include <xml_parse.hpp>

#include <console.hpp>
#include <iostream>

using namespace io;

void print_start_doc(std::ostream& cstr,const xml::s_event_stream_parser& s)
{
	xml::document_event e = s->parse_start_doc();
	if( s->is_error() )
		return;
	cstr<<"start document:\n";
	cstr<<"\tversion: " << e.version();
	cstr<<" encoding: " << e.encoding();
	cstr<<" standalone: " << (e.standalone() ? "yes" : "no") << std::endl;
}

void print_instr(std::ostream& cstr, const xml::s_event_stream_parser& s)
{
	xml::instruction_event e = s->parse_processing_instruction();
	if( s->is_error() ) return;
	cstr<<"processing instruction:\n";
	cstr<<"\ttarget:" <<e.target();
	cstr<<"\n\tdata: " << e.data()<<'\n';
	cstr.flush();
}

void print_start_element(std::ostream& cstr, const xml::s_event_stream_parser& s)
{
	xml::start_element_event e = s->parse_start_element();
	if( s->is_error() )
		return;
	cstr<<"Start element:\n";
	cstr<<"\tprefix:"<<e.name().prefix();
	cstr<<" name:"<< e.name().local_name();
	cstr<<" empty element:" << ( e.empty_element() ? " yes" : " no") <<  '\n';
	if( e.has_attributes() ) {
		cstr<<"\tattributes:\n";
		xml::start_element_event::iterator i = e.attr_begin();
		do {
			cstr<<"\t\tname: " << i->name();
			cstr<<" value: " << i->value() << '\n';
			++i;
		} while(i != e.attr_end() );
	}
	cstr.flush();
}

void print_end_element(std::ostream& cstr, const xml::s_event_stream_parser& s)
{
	xml::end_element_event e = s->parse_end_element();
	if(s->is_error()) return;
	cstr<<"End element:\n\tprefix:" << e.name().prefix();
	cstr<<" name:" << e.name().local_name() << std::endl;
}

void print_event(std::ostream& cstr,const xml::s_event_stream_parser& s)
{
	switch( s->current_event() ) {
	case xml::event_type::start_document:
		print_start_doc(cstr, s );
		break;
	case xml::event_type::processing_instruction:
		print_instr( cstr, s );
		break;
	case xml::event_type::start_element:
		print_start_element( cstr, s );
		break;
	case xml::event_type::end_element:
		print_end_element( cstr, s );
		break;
	}
}

static  void log_chars(std::ostream& cstr,const char* msg, const char* chars)
{
	cstr<< msg << '\n' << chars << std::endl;
}

static void on_terminate() noexcept
{
	io::exit_with_current_error();
}


int main(int argc, const char** argv)
{

	std::set_terminate( on_terminate );

	std::ostream& cout = io::console::out_stream();
	std::ostream& cerr = io::console::error_stream();
	if(argc < 2) {
		cout<< "XML parsing example\n Usage:\t xmlparse <xmlfile>" <<std::endl;
		return 0;
	}
	std::error_code ec;
	file sf = file::get(ec, argv[1] );
	io::check_error_code( ec );
	if( !sf.exist() ) {
		cerr << sf.path() << " can not be found" << std::endl;
		return ec.value();
	}

	xml::s_source src = xml::source::create(ec, sf.open_for_read(ec) );
	//xml::s_source src = xml::source::create(ec, sf.open_for_read(ec), code_pages::UTF_16LE );
	io::check_error_code( ec );
	xml::s_event_stream_parser xs = xml::event_stream_parser::open(ec, src);
	io::check_error_code( ec );

	xml::state state;
	do {
		state = xs->scan_next();
		switch(state.current) {
		case xml::state_type::initial:
		case xml::state_type::eod:
			break;
		case xml::state_type::event:
			print_event(cout,xs);
			break;
		case xml::state_type::comment:
			log_chars(cout, "Comment: ", xs->read_comment().data() );
			//xs->skip_comment();
			break;
		case xml::state_type::cdata:
			log_chars(cout,"CDATA: ", xs->read_cdata().data() );
			break;
		case xml::state_type::characters:
			log_chars(cout,"Characters: ", xs->read_chars().data() );
			break;
		case xml::state_type::dtd:
			log_chars(cout,"DTD:", xs->read_dtd().data() );
			//xs->skip_dtd();
			break;
		}
	} while(xml::state_type::eod != state.current);
	if( xs->is_error() ) {
		xs->get_last_error(ec);
		cerr<< "XML error [" << xs->row() << ',' << xs->col() << "] " << ec.message() << std::endl;
	}
	return 0;
}
