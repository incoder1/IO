
#ifdef _WIN32
#	if _WIN32_WINNT < 0x0600
#		undef  _WIN32_WINNT
#		define WINVER 0x0600
#		define _WIN32_WINNT 0x0600
#	endif // _WIN32_WINNT
#endif

#include <algorithm>
#include <iostream>

#include <files.hpp>
#include <xml_parse.hpp>

namespace io {

namespace xml {

class reader {
public:
	start_element_event next_tag_begin(std::error_code& ec) noexcept;
	end_element_event next_tag_end(std::error_code& ec) noexcept;
	const_string next_characters(std::error_code& ec) noexcept;
	explicit reader(s_event_stream_parser&& parser) noexcept:
		parser_( std::forward<s_event_stream_parser>(parser) ),
		state_()
	{}
private:
	inline bool parse_error(std::error_code& ec) noexcept {
		if(ec)
			return true;
		if( error::ok != state_.ec ) {
			parser_->get_last_error(ec);
			return true;
		}
		return false;
	}

	void to_next_state(std::error_code& ec) noexcept;
private:
	s_event_stream_parser parser_;
	state state_;
};

void reader::to_next_state(std::error_code& ec) noexcept
{
	bool done = false;
	do {
		state_ = parser_->scan_next();
		switch(state_.current) {
		case state_type::initial:
			break;
		case state_type::eod:
			ec = std::make_error_code(error::invalid_state);
			break;
		case state_type::comment:
			parser_->skip_comment();
			break;
		case state_type::dtd:
			parser_->skip_dtd();
			break;
		case state_type::characters:
			if( !parser_->read_chars().blank() ) {
				// error non blank characters between tags
				ec = std::make_error_code(error::invalid_state);
			}
			break;
		default:
			done = true;
			break;
		}
	}
	while( !parse_error(ec) && !done );
}



start_element_event reader::next_tag_begin(std::error_code& ec) noexcept
{
	if(state_type::event != state_.current)
		to_next_state(ec);
	bool scaning = true;
	while( scaning && !parse_error(ec) ) {
		switch( parser_->current_event() ) {
		case event_type::start_element:
			scaning = false;
			break;
		case event_type::end_element:
			ec = make_error_code(error::invalid_state);
			scaning = false;
			break;
		case event_type::start_document:
			parser_->parse_start_doc();
			to_next_state(ec);
			break;
		case event_type::processing_instruction:
			parser_->parse_processing_instruction();
			to_next_state(ec);
			break;
		}
	}
	if(!ec) {
		start_element_event ret = parser_->parse_start_element();
		if( error::ok != state_.ec )
			parser_->get_last_error(ec);
		state_ = parser_->scan_next();
		return std::move(ret);
	}
	return start_element_event();
}

end_element_event reader::next_tag_end(std::error_code& ec) noexcept
{
	if(state_type::event != state_.current)
		to_next_state(ec);
	if(event_type::end_element != parser_->current_event()) {
		ec = make_error_code(error::invalid_state);
		return end_element_event();
	}
	end_element_event ret = parser_->parse_end_element();
	if( error::ok != state_.ec )
		parser_->get_last_error(ec);
	else
		state_ = parser_->scan_next();
	return std::move(ret);
}

const_string reader::next_characters(std::error_code& ec) noexcept
{
	if(state_type::characters != state_.current && state_type::cdata != state_.current) {
		ec = make_error_code(error::invalid_state);
		return const_string();
	}
	byte_buffer buff = byte_buffer::allocate(ec, 16);
	if(ec)
		return const_string();
	std::size_t s = 0;
	const_string chars;
	while(state_type::characters == state_.current || state_type::cdata == state_.current) {
		if(state_type::characters == state_.current)
			chars = parser_->read_chars();
		else
			chars = parser_->read_cdata();
		if( parser_->is_error() ) {
			parser_->get_last_error(ec);
			return const_string();
		}
		s = chars.size();
		if( buff.available() < s && !buff.extend( s + 1 ) ) {
			parser_->get_last_error(ec);
			return const_string();
		}
		buff.put( chars.data() );
		state_ = parser_->scan_next();
	}
	if( parser_->is_error() ) {
		parser_->get_last_error(ec);
		return const_string();
	}
	buff.flip();
	return const_string(buff.position().cdata(), buff.last().cdata());
}

} // namespace xml

} // namespace io


int main()
{
	io::file sf("app-config.xml");
	std::error_code ec;

	io::xml::s_source src = io::xml::source::create(ec, sf.open_for_read(ec) );
	io::check_error_code( ec );
	io::xml::s_event_stream_parser psr = io::xml::event_stream_parser::open(ec, std::move(src) );
	io::check_error_code( ec );

	io::xml::reader rd( std::move(psr) );

	io::xml::start_element_event bev;
	io::xml::end_element_event eev;
	for(unsigned int i=0; i < 3; i++) {
		bev = rd.next_tag_begin(ec);
		io::check_error_code( ec );
		std::cout <<  bev.name().local_name() << '\n';
		if( bev.has_attributes() ) {
			std::for_each(bev.attr_begin(), bev.attr_end(), [] (const io::xml::attribute& attr) {
				std::cout<< '\t' <<  attr.name() << ' ' << attr.value() << '\n';
			} );
		}
	}
	std::cout.flush();

	for(unsigned c =0; c < 2; c++) {

		bev = rd.next_tag_begin(ec);
		std::cout << bev.name().local_name() << std::endl;
		std::for_each(bev.attr_begin(), bev.attr_end(), [] (const io::xml::attribute& attr) {
				std::cout<< '\t' <<  attr.name() << ' ' << attr.value() << std::endl;
		} );

		io::const_string ch;
		for(unsigned i=0; i < 2; i++) {
			bev = rd.next_tag_begin(ec);
			std::cout << bev.name().local_name() << std::endl;
			io::check_error_code( ec );
			ch = rd.next_characters(ec);
			io::check_error_code( ec );
			std::cout<< ch << std::endl;
			eev = rd.next_tag_end(ec);
			io::check_error_code( ec );
			std::cout << '/' << eev.name().local_name() << std::endl;
		}

		eev = rd.next_tag_end(ec);
		io::check_error_code( ec );
		std::cout << '/' << eev.name().local_name() << std::endl;

	}

	eev = rd.next_tag_end(ec);
	io::check_error_code( ec );
	std::cout << '/' << eev.name().local_name() << std::endl;

	eev = rd.next_tag_end(ec);
	io::check_error_code( ec );
	std::cout << '/' << eev.name().local_name() << std::endl;

	return 0;
}
