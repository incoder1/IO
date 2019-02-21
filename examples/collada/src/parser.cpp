#include "parser.hpp"
#include <cmath>

#include <type_traits>

namespace collada {


// helper free functions

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

static float parse_float(const io::const_string& val)
{
	char* s = const_cast<char*>( val.data() );
	return next_float(s, &s);
}

static void parse_vec4(const io::const_string& val, float * ret)
{
	char* s = const_cast<char*>( val.data() );
	for( std::size_t i = 0; i < 4; i++) {
		ret[i] = next_float(s, &s);
	}
}


static io::xml::s_event_stream_parser open_parser(io::s_read_channel&& src)
{
	std::error_code ec;
	io::xml::s_event_stream_parser ret = io::xml::event_stream_parser::open(ec, std::move(src) );
	io::check_error_code( ec );
	return ret;
}


template<class T>
static bool is_element(const T& e, const char* local_name) noexcept
{
	return e.name().equal("",local_name);
}


// parser
parser::parser(io::s_read_channel&& src) noexcept:
	xp_( open_parser( std::forward<io::s_read_channel>(src) ) ),
	ec_()
{
}

io::xml::state_type parser::to_next_state()
{
	io::xml::state_type ret;
	// Scan for next XML state
	for(;;) {
		ret = xp_->scan_next();
		if( io::xml::state_type::comment == ret) {
			xp_->skip_comment();
		}
		else if(io::xml::state_type::dtd == ret) {
			xp_->skip_dtd();
		}
		else {
			break;
		}
	}
	return ret;
}

void parser::check_eod(io::xml::state_type state,const std::string& msg)
{
	if(io::xml::state_type::eod == state) {
		if( xp_->is_error() ) {
			xp_->get_last_error(ec_);
			io::check_error_code(ec_);
		}
		else
			throw std::logic_error( msg );
	}
}



io::xml::event_type parser::to_next_tag_event(io::xml::state_type& state)
{
	io::xml::event_type ret;
	bool parsing = true;
	while( parsing ) {
		state = to_next_state();
		switch(state) {
		case io::xml::state_type::cdata:
			xp_->read_cdata();
			continue;
		case io::xml::state_type::characters:
			xp_->skip_chars();
			continue;
		case io::xml::state_type::eod:
			parsing = false;
		default:
			break;
		}
		// got event
		ret = xp_->current_event();
		// skip start doc and dtd
		switch(ret) {
		case io::xml::event_type::start_document:
			xp_->parse_start_doc();
			break;
		case io::xml::event_type::processing_instruction:
			xp_->parse_processing_instruction();
			break;
		default:
			parsing = false;
		}
	}

	if( state == io::xml::state_type::eod) {
		if( xp_->is_error() ) {
			xp_->get_last_error(ec_);
			io::check_error_code(ec_);
		}
		// invalid state
		ret = io::xml::event_type::start_document;
	}

	return ret;
}


io::xml::start_element_event parser::skip_to_tag(const char *local_name)
{
	io::xml::event_type et;
	io::xml::state_type state;
	io::xml::start_element_event ret;
	for(;;) {
		et = to_next_tag_event(state);
		check_eod(state, std::string("no tag ").append(local_name).append(" found") );


		if( io::xml::event_type::end_element == et) {
			xp_->parse_end_element();
		}
		else {
			ret = xp_->parse_start_element();
			if( is_element(ret, local_name) )
				break;
		}
	}
	return ret;
}

void parser::skip_element(const io::xml::start_element_event& e)
{
	using namespace io::xml;
	if( ! e.empty_element() ) {
		qname name = e.name();
		event_type et;
		start_element_event see;
		end_element_event ele;
		io::xml::state_type state;
		std::size_t nestin_level = 1;
		while( 0 != nestin_level) {
			et = to_next_tag_event(state);
			check_eod(state, "root element is unblanced");

			if( event_type::start_element == et ) {
				see = xp_->parse_start_element();
				if(name == see.name())
					++nestin_level;
			} else {
				ele = xp_->parse_end_element();
				if(name == ele.name() )
					--nestin_level;
			}
		}
	}
}

io::xml::start_element_event parser::to_next_tag_start(io::xml::state_type& state)
{
	io::xml::event_type et;
	do {
		et = to_next_tag_event(state);
		if(io::xml::state_type::eod == state)
			return io::xml::start_element_event();
		if( io::xml::event_type::end_element == et)
			xp_->parse_end_element();
	}
	while(io::xml::event_type::start_element != et );
	return xp_->parse_start_element();
}

io::const_string parser::get_tag_value()
{
	xp_->scan_next();
	return xp_->read_chars();
}

void parser::parse_pong(phong_effect& effect)
{
	static constexpr std::size_t TAGS_COUNT = 6;
	static constexpr const char* ERR_MSG = "pong is unbalanced";
	io::xml::qname name;
	io::xml::start_element_event e;
	io::xml::state_type state;
	for(std::size_t i=0; i < TAGS_COUNT; i++) {
		e = to_next_tag_start(state);
		check_eod(state,ERR_MSG);
		if( is_element(e,"ambient") ) {
			e = to_next_tag_start(state);
			check_eod(state,ERR_MSG);
			parse_vec4( get_tag_value(), effect.ambient );
		}
		else if( is_element(e,"diffuse") ) {
			e = to_next_tag_start(state);
			check_eod(state,ERR_MSG);
			parse_vec4( get_tag_value(), effect.diffuse );
		}
		else if( is_element(e,"emission") ) {
			e = to_next_tag_start(state);
			check_eod(state,ERR_MSG);
			parse_vec4( get_tag_value(), effect.emission );
		}
		else if( is_element(e,"specular") ) {
			e = to_next_tag_start(state);
			check_eod(state,ERR_MSG);
			parse_vec4( get_tag_value(), effect.specular );
		}
		else if( is_element(e,"shininess") ) {
			e = to_next_tag_start(state);
			check_eod(state,ERR_MSG);
			effect.shininess = parse_float( get_tag_value() );
		}
		else if( is_element(e,"index_of_refraction") ) {
			e = to_next_tag_start(state);
			check_eod(state,ERR_MSG);
			effect.refraction_index = parse_float( get_tag_value() );
		}
	}
}

// FIXME: dirty code for now, to be refactored
std::vector<material> parser::read_effect_library()
{

	std::vector<material> ret;

	io::xml::start_element_event e = skip_to_tag("effect");

	//while()

	material mat;

	mat.id = e.get_attribute("","id").first;

	// skip <profile_XXX> / <technique sid="XXX">
	// TODO: change
	skip_to_tag("phong");

	parse_pong( mat.effect );

	ret.push_back( mat );


	// skip all tags to end
	skip_element(e);

	return ret;
}

model parser::load()
{

	model ret;

	io::xml::state_type state;
	io::xml::start_element_event e;

	do {
		e = to_next_tag_start(state);
		if( is_element(e,"COLLADA") ) {
			continue;
		}

		if( is_element(e,"asset") ) {
			// not implemented
			skip_element(e);
		}
		else if( is_element(e,"library_animations") ) {
			// not implemented
			skip_element(e);
		}
		else if( is_element(e,"library_animation_clips") ) {
			// not implemented
			skip_element(e);
		}
		else if( is_element(e,"library_controllers") ) {
			// not implemented
			skip_element(e);
		}
		else if( is_element(e,"library_images") ) {
			// TODO: implement
			skip_element(e);
		}
		else if( is_element(e,"library_materials") ) {
			// TODO: check
			skip_element(e);
		}
		else if( is_element(e,"library_effects") ) {
			// TODO: implement fully
			ret.materials = read_effect_library();
		}
		else if( is_element(e,"library_geometries") ) {
			// TODO: implement
			skip_element(e);
		}
		else if( is_element(e,"library_visual_scenes") ) {
			// not implemented
			skip_element(e);
		}
		else if( is_element(e,"library_lights") ) {
			// not implemented
			skip_element(e);
		}
		else if( is_element(e,"library_cameras") ) {
			skip_element(e);
		}
		else if( is_element(e, "library_nodes") ) {
			// TODO: implement
			skip_element(e);
		}
		else {
			skip_element(e);
		}
	}
	while( state != io::xml::state_type::eod );

	if( xp_->is_error() ) {
		xp_->get_last_error( ec_ );
		io::check_error_code( ec_ );
	}

	return ret;
}

parser::~parser() noexcept
{
}


} // namespace collada
