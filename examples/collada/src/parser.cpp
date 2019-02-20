#include "parser.hpp"
#include <cmath>


namespace collada {


static io::xml::s_event_stream_parser open_parser(io::s_read_channel&& src)
{
	std::error_code ec;
	io::xml::s_event_stream_parser ret = io::xml::event_stream_parser::open(ec, std::move(src) );
	io::check_error_code( ec );
	return ret;
}

parser::parser(io::s_read_channel&& src) noexcept:
	xp_( open_parser( std::forward<io::s_read_channel>(src) ) ),
	ec_()
{
}


typedef io::xml::lexical_cast_traits<float> float_cast;

static float next_float(const char* str,char** endp)
{
	while( std::isspace(*str) )
		++str;
	float ret = NAN;
	if('\0' != *str)
		ret = std::strtof(str, endp);
	else
		*endp = nullptr;
	return ret;
}


static io::scoped_arr<float> parse_string_list(const io::const_string& val, std::size_t size)
{
	if( ! val.blank() ) {
		io::scoped_arr<float> ret( size );
		char *s = const_cast<char*>( val.data() );
		std::size_t i = 0;
		while( nullptr !=  s) {
			ret[i++] = next_float(s, &s);
		}
		return ret;
	}
	return io::scoped_arr<float>();
}

io::xml::state_type parser::to_next_state()
{
	io::xml::state_type ret;
	// Scan for next XML state
	for(;;) {
		ret = xp_->scan_next();
		if( io::xml::state_type::comment == ret) {
			xp_->skip_comment();
		} else if(io::xml::state_type::dtd == ret) {
			xp_->skip_dtd();
		} else {
			break;
		}
	}
	return ret;
}

io::xml::event_type parser::to_next_tag_event()
{
	io::xml::state_type state;
	do {
		state = to_next_state();
		if( io::xml::state_type::event == state ) {
			// got event
			io::xml::event_type ret = xp_->current_event();
			// skip start doc and dtd
			if(io::xml::event_type::start_document == ret) {
				xp_->parse_start_doc();
				continue;
			}
			else if(io::xml::event_type::processing_instruction == ret) {
				xp_->parse_processing_instruction();
				continue;
			}
			return ret;
		} else if( io::xml::state_type::cdata == state )
			xp_->read_cdata();
		else if( io::xml::state_type::characters == state)
			xp_->skip_chars();
		else if( io::xml::state_type::eod == state )
			break;
	}
	while( io::xml::state_type::eod != state );

	if( xp_->is_error() ) {
		if( xp_->is_error() ) {
			xp_->get_last_error(ec_);
			io::check_error_code( ec_ );
		}
	}

	throw std::logic_error("Wrong XML sequence");
}


io::xml::start_element_event parser::skip_to_tag(const char* prefix, const char *local_name)
{
	io::xml::event_type et;
	io::xml::start_element_event ret;
	for(;;) {
		et = to_next_tag_event();
		if( io::xml::event_type::end_element == et) {
            xp_->parse_end_element();
		} else {
			ret = xp_->parse_start_element();
			if( ret.name().equal(prefix,local_name) )
				break;
		}
	}
	return ret;
}

static void parse_color(const io::const_string& val, float * ret)
{
	char* s = const_cast<char*>( val.data() );
	for( std::size_t i = 0; i < 4; i++) {
		ret[i] = next_float(s, &s);
	}
}

void parser::parse_pong(phong_effect& effect)
{
	io::xml::qname name;
	io::xml::event_type et;
	io::xml::start_element_event e;
	for(std::size_t i=0; i < 6; i++) {

		do {
			et = to_next_tag_event();
			if( io::xml::event_type::end_element == et)
				xp_->parse_end_element();
		} while(io::xml::event_type::start_element != et );

		e = xp_->parse_start_element();

		name = e.name();

		if( name.equal("","ambient") ) {
			e = skip_to_tag("","color");
			xp_->scan_next();
			parse_color( xp_->read_chars(), effect.ambient );
		}
		else if( name.equal("","diffuse") ) {
			e = skip_to_tag("","color");
			xp_->scan_next();
			parse_color( xp_->read_chars(), effect.diffuse );
		}
		else if( name.equal("","emission") ) {
			e = skip_to_tag("","color");
			xp_->scan_next();
			parse_color( xp_->read_chars(), effect.emission );
		}
		else if( name.equal("","specular") ) {
			e = skip_to_tag("","color");
			xp_->scan_next();
			parse_color( xp_->read_chars(), effect.specular );
		}
		else if( name.equal("","shininess") ) {
			e = skip_to_tag("","float");
			xp_->scan_next();
			io::const_string val = xp_->read_chars();
			char *s = const_cast<char*>( val.data() );
			effect.shininess = next_float( s, &s);
		}
		else if( name.equal("","index_of_refraction") ) {
			e = skip_to_tag("","float");
			xp_->scan_next();
			io::const_string val = xp_->read_chars();
			char *s = const_cast<char*>( val.data() );
			effect.refraction_index = next_float( s, &s);
		}
	}

}

// FIXME: dirty code for now, to be refactored
std::vector<material> parser::read_materials()
{
	// skip to <library_effects>
	io::xml::start_element_event e = skip_to_tag("","library_effects");

	if( !e )
		return std::vector<material>();

	std::vector<material> ret;

	e = skip_to_tag("","effect");
	//while( e ) {

	material mat;

	mat.id = e.get_attribute("","id").first;

	// skip <profile_XXX> / <technique sid="XXX">
	// TODO: change
	e = skip_to_tag("","phong");

	parse_pong( mat.effect );

	ret.push_back( mat );

	return ret;
}

model parser::load()
{
	model ret;
	ret.materials = read_materials();
	return ret;
}

parser::~parser() noexcept
{
}


} // namespace collada
