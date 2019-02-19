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
	if('\0' != *str) {
		ret = float_cast::from_string( str );
		while( std::isdigit(*str) )
			++str;
		if('\0' != *str)
			*endp = const_cast<char*>(str);
		else
			*endp = nullptr;
	}
	else {
		*endp = nullptr;
	}
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

io::xml::start_element_event parser::skip_to_tag(const char* prefix, const char *local_name)
{
	io::xml::start_element_event ret = to_next_tag();
	do {
		ret = to_next_tag();
	}
	while( !ret.name().equal(prefix,local_name) );
	// no such element found
	return ret;
}


io::xml::start_element_event parser::to_next_tag()
{
	using namespace io::xml;
	state_type state;
	do {
		// Scan for next XML state
		state = xp_->scan_next();
		switch(state) {
		case state_type::initial:
			continue;
		case state_type::eod:
		case state_type::event:
			break;
		case state_type::comment:
			xp_->skip_comment();
			continue;
		case state_type::cdata:
			xp_->read_cdata();
			continue;
		case state_type::characters:
			xp_->skip_chars();
			continue;
		case state_type::dtd:
			xp_->skip_dtd();
			continue;
			break;
		}
		if(  state == state_type::eod )
			break;
		// got event
		switch( xp_->current_event() ) {
		// this is start document event, i.e. XML prologue section
		case event_type::start_document:
			xp_->parse_start_doc();
			continue;
		// this is an XML processing instruction event
		case event_type::processing_instruction:
			xp_->parse_processing_instruction();
			continue;
		// this is end element event
		case event_type::end_element:
			xp_->parse_end_element();
			continue;
		// this is start element event
		case event_type::start_element:
			return xp_->parse_start_element();
		}
	}
	while( state_type::eod != state );
	if( xp_->is_error() ) {
		xp_->get_last_error(ec_);
		io::check_error_code( ec_ );
	}
	// no such element found
	return io::xml::start_element_event();
}

static void parse_color(const io::const_string& val, float * ret)
{
	char* s[1] = { const_cast<char*>( val.data() ) };
	std::size_t i = 0;
	while( nullptr != s[0] )
		ret[i++] = next_float(s[0], s);
}

// FIXME: dirty code for now, to be refactored
std::vector<material> parser::read_materials()
{
	// skip to <library_effects>
	io::xml::start_element_event e = skip_to_tag("","library_effects");

	if( !e )
		return std::vector<material>();

	e = skip_to_tag("","phong");

	if(!e)
		throw std::runtime_error("This COLLADA feature is not yet implemented");

	std::vector<material> ret;

	material mat;
	io::xml::qname name;
	for(std::size_t i=0; i < 5; i++) {
		name = to_next_tag().name();
		if( name.equal("","ambient") ) {
			e = skip_to_tag("","color");
			xp_->scan_next();
			parse_color( xp_->read_chars() , mat.effect.ambient );
		}
		else if( name.equal("","diffuse") ) {
			e = skip_to_tag("","color");
			xp_->scan_next();
			parse_color( xp_->read_chars(), mat.effect.diffuse );
		}
		else if( name.equal("","emission") ) {
			e = skip_to_tag("","color");
			xp_->scan_next();
			parse_color( xp_->read_chars(), mat.effect.emission );
		} else if( name.equal("","specular") ) {
			e = skip_to_tag("","color");
			xp_->scan_next();
			parse_color( xp_->read_chars(), mat.effect.specular );
		} else if( name.equal("","shininess") ) {
			e = skip_to_tag("","float");
			xp_->scan_next();
			mat.effect.shininess = float_cast::from_string( xp_->read_chars().data() );
		}
	}

	ret.emplace_back(mat);

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
