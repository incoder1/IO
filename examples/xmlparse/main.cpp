#include <files.hpp>
#include <text.hpp>
#include <xml_parse.hpp>
#include <iostream>

using namespace io;

void print_start_doc(const xml::s_event_stream_parser& s)
{
	xml::document_event e = s->parse_start_doc();
	if( s->is_error() ) return;
	std::cout<<"start document:\n";
	std::cout<<"\tversion: "<< e.version() ;
	std::cout<<" encoding: " << e.encoding() ;
	std::cout<<" standalone: " << (e.standalone() ? "yes" : "no") << '\n';
}

void print_instr(const xml::s_event_stream_parser& s)
{
	xml::instruction_event e = s->parse_processing_instruction();
	if( s->is_error() ) return;
	std::cout<<"processing instruction:\n";
	std::cout<< "\ttarget:" <<e.target();
	std::cout<<"\n\tdata: " << e.data()<<'\n';
	std::cout.flush();
}

void print_start_element(const xml::s_event_stream_parser& s)
{
	xml::start_element_event e = s->parse_start_element();
	if( s->is_error() ) return;
	std::cout<<"Start element:\n";
	std::cout<<"\tprefix:"<<e.name().prefix();
	std::cout<<" name:"<< e.name().local_name();
	std::cout<<" empty element:" << ( e.empty_element() ? " yes" : " no") <<  "\n";
	if( e.has_attributes() ) {
		std::cout<<"\tattributes:\n";
		xml::start_element_event::iterator i = e.attr_begin();
		do {
			std::cout<<"\t\tname: " << i->name();
			std::cout<<" value: " << i->value() << '\n';
			++i;
		} while(i != e.attr_end() );
	}
	std::cout.flush();
}

void print_end_element(const xml::s_event_stream_parser& s)
{
	xml::end_element_event e = s->parse_end_element();
	if(s->is_error()) return;
	std::cout<<"End element:\n\tprefix:" << e.name().prefix();
	std::cout<<" name:" << e.name().local_name() << '\n';
	std::cout.flush();
}

void print_event(const xml::s_event_stream_parser& s)
{
	switch( s->current_event() ) {
	case xml::event_type::start_document:
		print_start_doc( s );
		break;
	case xml::event_type::processing_instruction:
		print_instr( s );
		break;
	case xml::event_type::start_element:
		print_start_element( s );
		break;
	case xml::event_type::end_element:
		print_end_element( s );
		break;
	}
}

void log_chars(const char* msg, const char* chars)
{
	std::cout<< msg << '\n' << chars << '\n';
}

void check_system_error(std::error_code& ec) {
	if(ec) {
        std::cerr<< ec.message() << std::endl;
        std::exit( ec.value() );
	}
}


int main(int argc, const char** argv)
{
	if(argc < 2) {
		std::cout<< "XML parsing example\n Usage:\t xmlparse <xmlfile>" <<std::endl;
		return 0;
	}
	std::error_code ec;
	file sf = file::get(ec, argv[1] );
	check_system_error(ec);
	if(!sf.exist()) {
		std::wcerr<<"file " << sf.name() << " is not exist"<<std::endl;
		return -2;
	}

	xml::s_source src = xml::source::create(ec, sf.open_for_read(ec), code_pages::UTF_8 );
	check_system_error(ec);
	xml::s_event_stream_parser xs = xml::event_stream_parser::open(ec, src);
	check_system_error(ec);

	xml::state state;
	do {
		state = xs->scan_next();
		switch(state.current) {
		case xml::state_type::initial:
		case xml::state_type::eod:
			break;
		case xml::state_type::event:
			print_event(xs);
			break;
		case xml::state_type::comment:
			log_chars("Comment: ", xs->read_comment().data() );
			//xs->skip_comment();
			break;
		case xml::state_type::cdata:
			log_chars("CDATA: ", xs->read_cdata().data() );
			break;
		case xml::state_type::characters:
			log_chars("Characters: ", xs->read_chars().data() );
			break;
		case xml::state_type::dtd:
			log_chars("DTD:", xs->read_dtd().data() );
			//xs->skip_dtd();
			break;
		}
	} while(xml::state_type::eod != state.current);
	if( xs->is_error() ) {
		xs->get_last_error(ec);
		std::cerr<< "XML error [" << xs->row() << ',' << xs->col() << "] " << ec.message() << std::endl;
	}
	return 0;
}
