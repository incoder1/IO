#include "stdafx.hpp"
#include "parser.hpp"

#include <cmath>

namespace collada {


// xml parse helper free functions
static bool is_start_element(io::xml::event_type et) noexcept
{
	return io::xml::event_type::start_element == et;
}

static float next_float(const char* str,char** endp)
{
	float ret = NAN;
	if( io_likely( '\0' != *str) )
		ret = std::strtof(str, endp);
	else
		*endp = nullptr;
	return ret;
}

static unsigned int next_uint(const char* str,char** endp)
{
	unsigned int ret = 0;
	if( io_likely( '\0' != *str) ) {
		ret = std::strtoul(str, endp, 10);
	} else
		*endp = nullptr;
	return ret;
}

static std::size_t parse_sizet(const io::const_string& str)
{
	typedef io::xml::lexical_cast_traits<std::size_t> size_t_cast;
	const char *s = str.data();
	return size_t_cast::from_string( s, &s);
}


static bool parse_bool(const io::const_string& val)
{
	typedef io::xml::lexical_cast_traits<bool> bool_cast;
	const char* str =  val.data();
	while( std::isspace(*str) )
		++str;
	switch(*str) {
	case '\0':
	case '0':
		return false;
	case '1':
		return true;
	default:
		return bool_cast::from_string(str, &str);
	}
}

static std::size_t parse_string_array(const io::const_string& val,const std::size_t size, float* const data)
{
	std::size_t ret = 0;
	if( ! val.blank() ) {
		// parse space separated list of floats
		char *s = const_cast<char*>( val.data() );
		for(std::size_t i = 0; (i < size); i++) {
			if( io_unlikely(nullptr == s) )
				break;
			data[i] = next_float(s, &s);
			if(  io_unlikely( std::isnan(data[i]) ) )
				break;
			++ret;
		}
	}
	return ret;
}

static unsigned_int_array parse_string_array(const io::const_string& val)
{
	//std::vector<unsigned int> tmp;
	std::size_t size = 0;
	char* s = const_cast<char*>( val.data() );
	do {
       	while(std::isspace(*s)) {
			++s;
		}
		if( std::isdigit(*s) ) {
			++size;
			while( std::isdigit(*s) ) {
				++s;
			}
		}
	} while( '\0' != *s);
	if(0 != size) {
		unsigned_int_array ret( size );
		s = const_cast<char*>( val.data() );
		for(std::size_t i=0; i < size; i++) {
			ret[i] = next_uint(s,&s);
		}
		return ret;
	}
	return unsigned_int_array();
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

// parser
parser::parser(io::s_read_channel&& src) noexcept:
	xp_( open_parser( std::forward<io::s_read_channel>(src) ) )
{
	// pre-cache basic strings
	// for faster parsing root elements
#define CACHE_STR(__x) __x##_ = xp_->precache(#__x)
	CACHE_STR(accessor);
	CACHE_STR(effect);
	CACHE_STR(float_array);
	CACHE_STR(input);
	CACHE_STR(geometry);
	CACHE_STR(node);
	CACHE_STR(mesh);
	CACHE_STR(visual_scene);
	CACHE_STR(vertices);
	CACHE_STR(param);
	CACHE_STR(source);
	//CACHE_STR(library_animations);
	//CACHE_STR(library_animation_clips);
	//CACHE_STR(library_images);
	CACHE_STR(library_materials);
	CACHE_STR(library_effects);
	CACHE_STR(library_geometries);
	CACHE_STR(library_visual_scenes);
	//CACHE_STR(library_lights);
	//CACHE_STR(library_nodes);
#undef CACHE_STR
}

io::xml::state_type parser::to_next_state() noexcept
{
	io::xml::state_type ret = io::xml::state_type::initial;
	// Scan for next XML state
	for(;;) {
		ret = xp_->scan_next();
		switch(ret) {
		case io::xml::state_type::dtd:
			xp_->skip_dtd();
			continue;
		case io::xml::state_type::comment:
			xp_->skip_comment();
			continue;
		default:
			break;
		}
		break;
	}
	return ret;
}

inline void parser::check_eod( io::xml::state_type state,const char* msg)
{
	if(io::xml::state_type::eod == state) {
		if( xp_->is_error() ) {
			std::error_code ec;
			xp_->get_last_error(ec);
			throw std::system_error(ec);
		}
		else
			throw std::logic_error( msg );
	}
}

inline void parser::check_eod(io::xml::state_type state,const std::string& msg)
{
	check_eod(state, msg.data() );
}

bool parser::is_end_element(io::xml::event_type et,const io::cached_string& local_name)
{
	using io::xml::event_type;
	return (event_type::end_element == et)
		   ? is_element( xp_->parse_end_element(), local_name)
		   : false;
}

inline bool parser::is_end_element(io::xml::event_type et,const io::xml::qname& tagname)
{
	return is_end_element(et, tagname.local_name() );
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
			std::error_code ec;
			xp_->get_last_error(ec);
			throw std::system_error(ec);
		}
		// invalid state
		ret = io::xml::event_type::start_document;
	}

	return ret;
}

void parser::skip_element(const io::xml::start_element_event& e)
{
	if( e.empty_element() )
		return;

	io::xml::qname name = e.name();
	std::string errmsg = std::string( name.local_name().data() );
	errmsg.append(" is unbalanced");

	io::xml::state_type state;
	io::xml::event_type et;
	io::xml::start_element_event sev;
	io::xml::end_element_event eev;
	std::size_t nestin_level = 1;
	do {
		et = to_next_tag_event(state);
		check_eod(state, errmsg);
		if( is_start_element(et) ) {
			sev = xp_->parse_start_element();
			// handle a situation when a tag embed
			// a nesting tag with the same name
			// more likely never happens
			if( io_unlikely( sev.name() == name ) )
				++nestin_level;
		} else {
			eev = xp_->parse_end_element();
			if( eev.name() == name)
				--nestin_level;
		}
	} while( 0 != nestin_level );
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
	switch( xp_->scan_next() ) {
	case io::xml::state_type::characters:
		return xp_->read_chars();
	case io::xml::state_type::cdata:
		return xp_->read_cdata();
	default:
		throw std::logic_error("Characters expected");
	}
}

void parser::parse_effect(effect& ef)
{
	static constexpr const char* ERR_MSG = "effect is unbalanced";

	io::xml::state_type state;
	io::xml::event_type et;
	io::xml::start_element_event sev;

	do {
		et = to_next_tag_event(state);
		check_eod(state, ERR_MSG);

		if( is_start_element(et ) )  {
			sev = xp_->parse_start_element();
			check_eod(state, ERR_MSG);

			if( is_element(sev,"profile_COMMON") ) {
				continue;
			}
			else if( is_element(sev,"technique") ) {
				continue;
			}
			else if( is_element(sev,"constant") ) {
				ef.value.shade = shade_type::constant;
			}
			else if( is_element(sev,"blinn") ) {
				ef.value.shade = shade_type::blinn_phong;
			}
			else if( is_element(sev,"phong") ) {
				ef.value.shade = shade_type::phong;
			}
			else if( is_element(sev,"lambert") ) {
				ef.value.shade = shade_type::lambert;
			}
			else if( is_element(sev,"ambient") ) {
				sev = to_next_tag_start(state);
				check_eod(state,ERR_MSG);
				parse_vec4( get_tag_value(), ef.value.mat.ambient );
			}
			else if( is_element(sev,"diffuse") ) {
				sev = to_next_tag_start(state);
				check_eod(state,ERR_MSG);
				parse_vec4( get_tag_value(), ef.value.mat.diffuse );
			}
			else if( is_element(sev,"emission") ) {
				sev = to_next_tag_start(state);
				check_eod(state,ERR_MSG);
				parse_vec4( get_tag_value(), ef.value.mat.emission );
			}
			else if( is_element(sev,"specular") ) {
				sev = to_next_tag_start(state);
				check_eod(state,ERR_MSG);
				parse_vec4( get_tag_value(), ef.value.mat.specular );
			}
			else if( is_element(sev,"shininess") ) {
				sev = to_next_tag_start(state);
				check_eod(state,ERR_MSG);
				ef.value.mat.shininess = parse_float( get_tag_value() );
			}
			else if( is_element(sev,"index_of_refraction") ) {
				sev = to_next_tag_start(state);
				check_eod(state,ERR_MSG);
				ef.value.mat.refraction_index = parse_float( get_tag_value() );
			}
			else if( is_element(sev,"reflective") ) {
				sev = to_next_tag_start(state);
				check_eod(state,ERR_MSG);
				parse_vec4( get_tag_value(), ef.value.reflect.color );
			}
			else if( is_element(sev,"reflectivity") ) {
				sev = to_next_tag_start(state);
				check_eod(state,ERR_MSG);
				ef.value.reflect.value = parse_float( get_tag_value() );
			}
			else if( is_element(sev,"transparent") ) {
				ef.value.transparent.used = true;
				io::const_string opaque = sev.get_attribute("","opaque").first;
				ef.value.transparent.rbg = opaque.equal("RGB_ZERO") || opaque.equal("RGB_ONE");
				ef.value.transparent.invert = opaque.equal("RGB_ZERO") || opaque.equal("A_ZERO");
				sev = to_next_tag_start(state);
				check_eod(state,ERR_MSG);
				parse_vec4( get_tag_value(), ef.value.transparent.color);
			}
			else if( is_element(sev,"double_sided") ) {
				check_eod(state,ERR_MSG);
				ef.value.ext_3max.double_sided = parse_bool( get_tag_value() );
			}
			else if( is_element(sev,"bump") ) {
				sev = to_next_tag_start(state);
				check_eod(state,ERR_MSG);
				parse_vec4( get_tag_value(), ef.text.bump );
			}
			else if( is_element(sev,"wireframe") ) {
				sev = to_next_tag_start(state);
				check_eod(state,ERR_MSG);
				ef.value.ext_3max.wireframe = parse_bool( get_tag_value() );
			}
			else if( is_element(sev,"faceted") ) {
				sev = to_next_tag_start(state);
				check_eod(state,ERR_MSG);
				ef.value.ext_3max.faceted = parse_bool( get_tag_value() );
			}
		}
	}
	while( !is_end_element(et,effect_) );
}

void parser::parse_effect_library(model& md)
{
	static constexpr const char* ERR_MSG = "library_effects is unbalanced";
	io::xml::state_type state;
	io::xml::event_type et;
	io::xml::start_element_event sev;
	do {
		et = to_next_tag_event(state);
		check_eod(state, ERR_MSG);
		if( is_start_element(et) ) {
			sev = xp_->parse_start_element();
			check_eod(state, ERR_MSG);
			if( is_element(sev,effect_) ) {
				effect ef;
				std::memset(&ef, 0, sizeof(ef) );
				io::const_string id = sev.get_attribute("","id").first;
				parse_effect( ef );
				md.add_effect( std::move(id), std::move(ef) );
			}
		}

	}
	while( !is_end_element(et,library_effects_) );
}

static input_channel parse_input(const io::xml::start_element_event& e)
{
	input_channel ret;
	auto attr = e.get_attribute("","semantic");
	if(!attr.second)
		throw std::runtime_error("input semantic attribute is mandatory");
	io::const_string sematic = attr.first;
	if( sematic.equal("VERTEX") ) {
		ret.type = semantic_type::vertex;
	}
	else if( sematic.equal("POSITION") ) {
		ret.type = semantic_type::position;
	}
	else if( sematic.equal("NORMAL") ) {
		ret.type = semantic_type::normal;
	}
	else if( sematic.equal("TEXCOORD") ) {
		ret.type = semantic_type::texcoord;
	}
	else if( sematic.equal("COLOR") ) {
		ret.type = semantic_type::color;
	}
	else if( sematic.equal("TANGENT") ) {
		ret.type = semantic_type::tangent;
	}
	else if( sematic.equal("BITANGENT") ) {
		ret.type = semantic_type::bitangent;
	}
	attr =  e.get_attribute("","source");
	if(!attr.second)
		throw std::runtime_error("input source attribute is mandatory");
	// remove # at the begin
	io::const_string src = attr.first;
	ret.accessor_id = io::const_string( src.data()+1, src.size()-1 );

	// optional attributes
	attr = e.get_attribute("","offset");
	if(attr.second)
		ret.offset = parse_sizet( attr.first );
	attr = e.get_attribute("","set");
	if(attr.second)
		ret.set = parse_sizet( attr.first );

	return ret;
}

void parser::parse_accessor(s_accessor& acsr)
{
	static constexpr const char* ERR_MSG = "accessor is unbalanced";
	io::xml::state_type state;
	io::xml::event_type et;
	io::xml::start_element_event sev;
	do {
		et = to_next_tag_event(state);
		check_eod(state, ERR_MSG);
		if( is_start_element(et) ) {
			sev = xp_->parse_start_element();
			check_eod(state, ERR_MSG);
			if( is_element(sev,param_) ) {
				parameter p;
				auto attr = sev.get_attribute("","name");
				if(attr.second)
					p.name = attr.first;
				attr = sev.get_attribute("","type");
				if(!attr.second)
					throw std::runtime_error("type attribute expected");
				io::const_string type = attr.first;
				p.presision = type.equal("float") ? presision::float32_t : presision::double64_t;
				acsr->add_parameter( std::move(p) );
			}
		}
	}
	while( !is_end_element(et,accessor_) );
}


void parser::parse_source(const s_source& src)
{

	static constexpr const char* ERR_MSG = "source is unbalanced";

	io::xml::state_type state;
	io::xml::event_type et;
	io::xml::start_element_event sev;
	do {
		et = to_next_tag_event(state);
		check_eod(state, ERR_MSG);
		if( is_start_element(et) ) {
			sev = xp_->parse_start_element();
			check_eod(state, ERR_MSG);
			if( sev.empty_element() || is_element(sev, "technique_common") ) {
				continue;
			}
			else if( is_element(sev,accessor_) ) {
				auto attr = sev.get_attribute("","source");
				io::const_string source_id;
				if(attr.second) {
					io::const_string ref = attr.first;
					// remove change #reference to reference
					source_id = io::const_string( ref.data()+1, ref.size() - 1 );
				} else
					throw std::runtime_error("accessor source attribute is mandatory");
				std::size_t count = parse_sizet( sev.get_attribute("","count").first );
				std::size_t stride = parse_sizet( sev.get_attribute("","stride").first );
				s_accessor acsr( new accessor( std::move(source_id), count, stride ) );
				parse_accessor(acsr);
				src->add_accessor( std::move(acsr) );
			}
			else if( is_element(sev, float_array_) ) {
				io::const_string id;
				std::size_t data_size = 0;
				auto attr = sev.get_attribute("","id");
				if(attr.second)
					const io::const_string id = attr.first;
				else
					throw std::runtime_error("float_array id attribute is mandatory");
				attr = sev.get_attribute("","count");
				if(attr.second)
					data_size = parse_sizet( attr.first );
				else
					throw std::runtime_error("float_array count attribute is mandatory");
				io::const_string data_str = get_tag_value();
				float_array data(data_size);
				std::size_t actual_size = parse_string_array( data_str, data_size, const_cast<float*>( data.get() ) );
				if( io_likely(actual_size == data_size ) ) {
					src->add_float_array( std::move(id), std::move(data) );
				} else {
					std::string msg("float_array ");
					msg.append( std::to_string(data_size) );
					msg.append(" elements expected by got ");
					msg.append( std::to_string(actual_size) );
					msg.append(" mind be not a number value in the string");
					throw std::runtime_error( msg );
				}
			}
		}
	}
	while( !is_end_element(et, source_) );
}

void parser::parse_vertex_data(const s_mesh& m)
{
	io::xml::state_type state;
	io::xml::event_type et;
	io::xml::start_element_event sev;
	do {
		et = to_next_tag_event(state);
		check_eod(state, "vertices is unbalanced");
		if( is_start_element(et) ) {
			sev = xp_->parse_start_element();
			if( is_element(sev,input_) )
				m->add_input_channel( parse_input(sev) );
		}
	}
	while( !is_end_element(et,vertices_) );
}



bool parser::is_index_data(const io::xml::start_element_event& sev,primitive_type& pt) noexcept
{
	if( is_element(sev,"triangles") ) {
		pt = primitive_type::triangles;
		return true;
	} else if( is_element(sev,"lines") ) {
		pt = primitive_type::lines;
		return true;
	} else if( is_element(sev,"linestrips") ) {
		pt = primitive_type::linestrips;
		return true;
	} else if( is_element(sev,"polygons") ) {
		pt = primitive_type::polygons;
		return true;
	} else if( is_element(sev,"polylist") ) {
		pt = primitive_type::polylist;
		return true;
	} else if( is_element(sev,"trifans") ) {
		pt = primitive_type::trifans;
		return true;
	} else if( is_element(sev,"tristrips")) {
		pt = primitive_type::tristrips;
		return true;
	}
	return false;
}

void parser::parse_mesh(const s_mesh& m)
{
	static constexpr const char* ERR_MSG = "mesh is unbalanced";


	io::xml::state_type state;
	io::xml::event_type et;
	io::xml::start_element_event sev;
	primitive_type pt;
	do {
		et = to_next_tag_event(state);
		check_eod(state, ERR_MSG);
		if( is_start_element(et) ) {
			sev = xp_->parse_start_element();
			check_eod(state, ERR_MSG);
			if( is_element(sev, source_) ) {
				io::const_string id = sev.get_attribute("","id").first;
				s_source src( new source() );
				parse_source(src);
				m->add_source( std::move(id), std::move(src) );
			}
			else if( is_element(sev, vertices_) ) {
				m->set_vertex_id( std::move(sev.get_attribute("","id").first) );
				parse_vertex_data(m);
			} else if( is_index_data(sev,pt) ) {
				s_index_data idx = m->get_index();
				idx->set_primitives(pt);
				auto attr = sev.get_attribute("","count");
				if(!attr.second) {
					std::string msg( sev.name().local_name().data() );
					msg.append("  count attribute is mandatory");
					throw std::runtime_error(msg);
				}
				idx->set_count(  parse_sizet( attr.first ) );
			} else if( is_element(sev,"input") ) {
				m->add_input_channel( parse_input(sev) );
			} else if( is_element(sev,"p") ) {
				m->get_index()->set_indices( parse_string_array( get_tag_value() ) );
			}
		}
	}
	while( !is_end_element(et, mesh_) );
}

void parser::pase_geometry_library(model& md)
{
	static const char* ERR_MSG = "library_geometries is unbalanced";
	io::xml::state_type state;
	io::xml::event_type et;
	io::xml::start_element_event sev;
	io::const_string geometry_id;
	io::const_string geometry_name;
	do {
		et = to_next_tag_event(state);
		check_eod(state, ERR_MSG);
		if( is_start_element(et) ) {
			sev = xp_->parse_start_element();
			check_eod(state, ERR_MSG);
			if( sev.empty_element() )
				continue;
			else if( is_element(sev,geometry_) ) {
				auto attr = sev.get_attribute("","id");
				if( !attr.second )
					throw std::runtime_error("geometry id argument is mandatory");
				geometry_id = attr.first;
				attr = sev.get_attribute("","name");
				geometry_name = attr.second ? attr.first : io::const_string(geometry_id);
			}
			else if( is_element(sev,mesh_) ) {
				s_mesh m( new mesh(std::move(geometry_name) ) );
				parse_mesh(m);
				md.add_mesh( std::move(geometry_id), std::move(m) );
			}
		}
	}
	while( !is_end_element(et, library_geometries_) );
}

void parser::parse_node(node& nd)
{
	static const char* ERR_MSG = "node is unbalanced";
	io::xml::state_type state;
	io::xml::event_type et;
	io::xml::start_element_event sev;
	do {
		et = to_next_tag_event(state);
		check_eod(state, ERR_MSG);
		if( is_start_element(et) ) {
			sev = xp_->parse_start_element();
			check_eod(state, ERR_MSG);
			if( is_element(sev,"instance_geometry") ) {
                auto attr = sev.get_attribute("","url");
                if(!attr.second)
                	throw std::runtime_error("instance_geometry url attribute is mandatory");
                io::const_string url = attr.first;
                nd.geo_ref.url = io::const_string( url.data()+1, url.size() - 1);
                attr = sev.get_attribute("","name");
                if( attr.second )
                	nd.geo_ref.name.swap( attr.first );
			} else if(is_element(sev,"instance_material") ) {
				auto attr = sev.get_attribute("","target");
				if( !attr.second )
					throw std::runtime_error("instance_material target attribute is mandatory");
				io::const_string target( attr.first.data()+1, attr.first.size() -1);
				nd.geo_ref.mat_ref.target.swap( target );
				attr = sev.get_attribute("","symbol");
				nd.geo_ref.mat_ref.symbol.swap( attr.first );
			}
		}
	} while( !is_end_element(et, node_) );
}

void parser::parse_visual_scene(s_scene& scn)
{
	static const char* ERR_MSG = "visual_scene is unbalanced";
	io::xml::state_type state;
	io::xml::event_type et;
	io::xml::start_element_event sev;
	do {
		et = to_next_tag_event(state);
		check_eod(state, ERR_MSG);
		if( is_start_element(et) ) {
			sev = xp_->parse_start_element();
			check_eod(state, ERR_MSG);
            if( is_element(sev, node_) ) {
				node nd;
				auto attr = sev.get_attribute("","id");
				if(!attr.second)
					throw std::runtime_error("node id attribute is mandatory");
				nd.id = std::move(attr.first);
				attr = sev.get_attribute("","name");
				if(attr.second)
					nd.name = std::move(attr.first);
				attr = sev.get_attribute("","type");
                if(attr.second && !attr.first.equal("type") )
                    nd.type = node_type::joint;
				else
					nd.type = node_type::node;
				parse_node(nd);
				scn->add_node( std::move(nd) );
            }
		}
	}
	while( !is_end_element(et, visual_scene_) );
}

void parser::library_visual_scenes(model& md)
{
	static const char* ERR_MSG = "library_visual_scenes is unbalanced";
	io::xml::state_type state;
	io::xml::event_type et;
	io::xml::start_element_event sev;
	do {
		et = to_next_tag_event(state);
		check_eod(state, ERR_MSG);
		if( is_start_element(et) ) {
			sev = xp_->parse_start_element();
			check_eod(state, ERR_MSG);
			if( is_element(sev,visual_scene_) ) {
				io::const_string id;
				io::const_string name;
				auto attr = sev.get_attribute("","id");
				if(attr.second)
					id = std::move(attr.first);
				attr = sev.get_attribute("","name");
				if(attr.second)
                    name = std::move(attr.first);
				s_scene scn( new scene( std::move(id), std::move(name) ) );
				parse_visual_scene(scn);
                md.set_scene( std::move(scn) );
			}
		}
	} while( !is_end_element(et,library_visual_scenes_) );
}


model parser::load()
{
	model ret;
	io::xml::state_type state;
	io::xml::start_element_event e = to_next_tag_start(state);
	check_eod(state, "Expecting COLLADA model file");
	if( ! is_element(e,"COLLADA") )
		throw std::runtime_error("Expecting COLLADA model file");
	do {
		e = to_next_tag_start(state);
		// nothing to do
		if( e.empty_element() )
			continue;
//		if( is_element(e,library_animations_) ) {
//			// not implemented
//			skip_element(e);
//		}
//		else if( is_element(e,library_animation_clips_) ) {
//			// not implemented
//			skip_element(e);
//		}
//		else if( is_element(e,library_images_) ) {
//			// TODO: implement
//			skip_element(e);
//		}
//		else if( is_element(e,library_materials_) ) {
//			// TODO: implement
//			skip_element(e);
//		}
		else if( is_element(e,library_effects_) ) {
			parse_effect_library(ret);
		}
		else if( is_element(e,library_geometries_) ) {
			pase_geometry_library(ret);
		}
		else if( is_element(e,library_visual_scenes_) ) {
			library_visual_scenes(ret);
//		}
//		else if( is_element(e,library_nodes_) ) {
//			// TODO: implement
//			skip_element(e);
		}
	}
	while( state != io::xml::state_type::eod );

	if( xp_->is_error() ) {
		std::error_code ec;
		xp_->get_last_error( ec );
		throw std::system_error(ec);
	}

	return ret;
}

parser::~parser() noexcept
{
}


} // namespace collada
