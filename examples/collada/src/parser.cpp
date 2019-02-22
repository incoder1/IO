#include "parser.hpp"

#include <cmath>
#include <type_traits>

namespace collada {

// FIXME: move collada model into separate module
// model
void model::add_effect(io::const_string&& id,effect&& e)
{
	effects_.emplace( std::forward<io::const_string>(id), std::make_shared<effect>( std::forward<effect>(e) ) );
}

std::shared_ptr<effect> model::find_effect(const char* id) noexcept
{
	effect_library_t::const_iterator it = effects_.find( io::const_string(id) );
	return effects_.cend() != it ? std::shared_ptr<effect>() : it->second;
}


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

static bool parse_bool(const io::const_string& val)
{
	typedef io::xml::lexical_cast_traits<bool> bool_cast;
	const char* str = val.data();
	while( std::isspace(*str) )
		++str;
	switch(*str) {
	case '\0':
	case '0':
		return false;
	case '1':
		return true;
	default:
		return bool_cast::from_string(str);
	}
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
	return e.name().local_name().equal( local_name );
}

template<class T>
static bool is_element(const T& e,const io::cached_string& local_name) noexcept
{
	// check with cache miss
	return local_name == e.name().local_name();
}


// parser
parser::parser(io::s_read_channel&& src) noexcept:
	xp_( open_parser( std::forward<io::s_read_channel>(src) ) ),
	ec_()
{
	// pre-cache basic strings
	// for faster parsing root elements
#define CACHE_STR(__x) __x##_ = xp_->precache(#__x)
	CACHE_STR(asset);
	CACHE_STR(effect);
	CACHE_STR(library_animations);
	CACHE_STR(library_animation_clips);
	CACHE_STR(library_controllers);
	CACHE_STR(library_images);
	CACHE_STR(library_materials);
	CACHE_STR(library_effects);
	CACHE_STR(library_geometries);
	CACHE_STR(library_visual_scenes);
	CACHE_STR(library_lights);
	CACHE_STR(library_cameras);
	CACHE_STR(library_nodes);
#undef CACHE_STR
}

io::xml::state_type parser::to_next_state()
{
	io::xml::state_type ret;
	// Scan for next XML state
	bool scan = true;
	while(scan) {
		ret = xp_->scan_next();
		switch(ret) {
		case io::xml::state_type::dtd:
			xp_->skip_dtd();
			break;
		case io::xml::state_type::comment:
			xp_->skip_comment();
			break;
		default:
			scan = false;
			break;
		}
	}
	return ret;
}

inline void parser::check_eod( io::xml::state_type state,const char* msg)
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

inline void parser::check_eod(io::xml::state_type state,const std::string& msg)
{
	check_eod(state, msg.data() );
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
	const std::string errmsg = std::string("no tag ").append(local_name).append(" found");
	for(;;) {
		et = to_next_tag_event(state);
		check_eod(state, errmsg );

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
		const std::string errmsg = std::string(name.local_name().data()).append(" is unbalanced found");
		event_type et;
		start_element_event see;
		end_element_event ele;
		io::xml::state_type state;
		std::size_t nestin_level = 1;
		while( 0 != nestin_level) {
			et = to_next_tag_event(state);
			check_eod(state, errmsg);

			if( event_type::start_element == et ) {
				see = xp_->parse_start_element();
				if(name == see.name())
					++nestin_level;
			}
			else {
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

void parser::parse_effect(effect& ef)
{
	static constexpr const char* ERR_MSG = "effect is unbalanced";

	io::xml::state_type state;
	io::xml::event_type et;
	io::xml::start_element_event sev;
	io::xml::end_element_event eev;

	for(;;) {
		et = to_next_tag_event(state);
		check_eod(state, ERR_MSG);
		if( io::xml::event_type::end_element == et) {
			eev = xp_->parse_end_element();
			if( is_element(eev, "effect") ) {
				break;
			}
			continue;
		}

		sev = xp_->parse_start_element();
		check_eod(state, ERR_MSG);

		if( is_element(sev,"profile_COMMON") ) {
			continue;
		} else if( is_element(sev,"technique") ) {
			continue;
		} else if( is_element(sev,"constant") ) {
			ef.shade = shade_type::constant;
		}
		else if( is_element(sev,"blinn") ) {
			ef.shade = shade_type::blinn_phong;
		}
		else if( is_element(sev,"phong") ) {
			ef.shade = shade_type::phong;
		}
		else if( is_element(sev,"lambert") ) {
			ef.shade = shade_type::lambert;
		}
		else if( is_element(sev,"ambient") ) {
			sev = to_next_tag_start(state);
			check_eod(state,ERR_MSG);
			parse_vec4( get_tag_value(), ef.tex_mat.ambient );
		}
		else if( is_element(sev,"diffuse") ) {
			sev = to_next_tag_start(state);
			check_eod(state,ERR_MSG);
			parse_vec4( get_tag_value(), ef.tex_mat.diffuse );
		}
		else if( is_element(sev,"emission") ) {
			sev = to_next_tag_start(state);
			check_eod(state,ERR_MSG);
			parse_vec4( get_tag_value(), ef.tex_mat.emission );
		}
		else if( is_element(sev,"specular") ) {
			sev = to_next_tag_start(state);
			check_eod(state,ERR_MSG);
			parse_vec4( get_tag_value(), ef.tex_mat.specular );
		}
		else if( is_element(sev,"shininess") ) {
			sev = to_next_tag_start(state);
			check_eod(state,ERR_MSG);
			ef.tex_mat.shininess = parse_float( get_tag_value() );
		}
		else if( is_element(sev,"index_of_refraction") ) {
			sev = to_next_tag_start(state);
			check_eod(state,ERR_MSG);
			ef.refraction_index = parse_float( get_tag_value() );
		} else if( is_element(sev,"reflective") ) {
			sev = to_next_tag_start(state);
			check_eod(state,ERR_MSG);
			parse_vec4( get_tag_value(), ef.reflect.color );
		} else if( is_element(sev,"reflectivity") ) {
			sev = to_next_tag_start(state);
			check_eod(state,ERR_MSG);
			ef.reflect.value = parse_float( get_tag_value() );
		} else if( is_element(sev,"transparent") ) {
			ef.transparent.used = true;
			io::const_string opaque = sev.get_attribute("","opaque").first;
			ef.transparent.rbg = opaque.equal("RGB_ZERO") || opaque.equal("RGB_ONE");
			ef.transparent.invert = opaque.equal("RGB_ZERO") || opaque.equal("A_ZERO");
			sev = to_next_tag_start(state);
			check_eod(state,ERR_MSG);
			parse_vec4( get_tag_value(), ef.transparent.color);
		} else if( is_element(sev,"double_sided") ) {
			check_eod(state,ERR_MSG);
			ef.double_sided = parse_bool( get_tag_value() );
		} else if( is_element(sev,"bump") ) {
			sev = to_next_tag_start(state);
			check_eod(state,ERR_MSG);
			parse_vec4( get_tag_value(), ef.text_bump );
		} else if( is_element(sev,"wireframe") ) {
			sev = to_next_tag_start(state);
			check_eod(state,ERR_MSG);
			ef.wireframe = parse_bool( get_tag_value() );
		} else if( is_element(sev,"faceted") ) {
			sev = to_next_tag_start(state);
			check_eod(state,ERR_MSG);
			ef.faceted = parse_bool( get_tag_value() );
		}
	}
}

void parser::parse_effect_library(model& md)
{

	io::xml::state_type state;
	io::xml::event_type et;
	io::xml::start_element_event sev;
	io::xml::end_element_event eev;

	for(;;) {
		et = to_next_tag_event(state);
		check_eod(state, "library_effects is unbalanced");
		if( io::xml::event_type::end_element == et) {
			eev = xp_->parse_end_element();
			if( is_element(eev, library_effects_) ) {
				break;
			}
			continue;
		}
		sev = xp_->parse_start_element();
		if( is_element(sev,"effect") ) {
			effect ef;
			io::const_string id = sev.get_attribute("","id").first;
			parse_effect( ef );
			md.add_effect( std::move(id), std::move(ef) );
		}
	}

}



model parser::load()
{

	model ret;

	io::xml::state_type state;
	io::xml::start_element_event e = to_next_tag_start(state);
	if( ! is_element(e,"COLLADA") ) {
		throw std::runtime_error("Expecting collada model file");
	}

	do {

		e = to_next_tag_start(state);
		// nothing to do
		if( e.empty_element() )
			continue;

		if( is_element(e,asset_) ) {
			// not implemented
			skip_element(e);
		}
		else if( is_element(e,library_animations_) ) {
			// not implemented
			skip_element(e);
		}
		else if( is_element(e,library_animation_clips_) ) {
			// not implemented
			skip_element(e);
		}
		else if( is_element(e,library_controllers_) ) {
			// not implemented
			skip_element(e);
		}
		else if( is_element(e,library_images_) ) {
			// TODO: implement
			skip_element(e);
		}
		else if( is_element(e,library_materials_) ) {
			// TODO: check
			skip_element(e);
		}
		else if( is_element(e,library_effects_) ) {
			// TODO: implement fully
			parse_effect_library(ret);
		}
		else if( is_element(e,library_geometries_) ) {
			// TODO: implement
			skip_element(e);
		}
		else if( is_element(e,library_visual_scenes_) ) {
			// not implemented
			skip_element(e);
		}
		else if( is_element(e,library_lights_) ) {
			// not implemented
			skip_element(e);
		}
		else if( is_element(e,library_cameras_) ) {
			skip_element(e);
		}
		else if( is_element(e,library_nodes_) ) {
			// TODO: implement
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
