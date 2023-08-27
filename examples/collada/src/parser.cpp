/*
 *
 * Copyright (c) 2016-2019
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#include "stdafx.hpp"
#include "parser.hpp"

#include <cstdlib>
#include <stdlib.h>

namespace collada {


// xml parse helper free functions
static bool is_start_element(io::xml::event_type et) noexcept
{
	return io::xml::event_type::start_element == et;
}

static float next_float(const char* str,const char* &endp) noexcept
{
	float ret = std::strtof(str, const_cast<char**> ( std::addressof(endp) ) );
	if(str == endp)
		endp = nullptr;
	return ret;
}

static unsigned int next_uint(const char* str,const char* &endp) noexcept
{
	unsigned int ret = std::strtoul(str, const_cast<char**> ( std::addressof(endp) ), 10);
	if(str == endp)
		endp = nullptr;
	return ret;
}

static std::size_t parse_sizet(const io::const_string& str) noexcept
{
	std::size_t ret;
	io::from_string(str,ret);
	return ret;
}

static bool not_empty(const char* str) noexcept
{
	return nullptr != str && '\0' != *str;
}


/// Parses space separated list of floats
static std::size_t parse_string_array(const io::const_string& val,float* const to, const std::size_t size) noexcept
{
	std::size_t ret = 0;
	if( ! val.blank() ) {
		const char *s = val.data();
		do {
			to[ret++] = next_float(s, s);
		}
		while( ret < size && not_empty(s) );
	}
	return ret;
}

// Regex like to count integers separated by spaces in a strings
static std::size_t count_string_numbers(const io::const_string& val) noexcept
{
	static constexpr const char* DIGIGTS = "0123456789";
	std::size_t ret = 0;
	const char* s = val.data();
	std::size_t len = 0;
	// count numbers
	do {
		// skip all whitespace characters and all digits
		// stop on next space after a last digit
		// i.e. count groups by (\s+\d+) regular expression
		len = io_strspn( io::skip_spaces(s), DIGIGTS );
		// no expression found, stop counting
		if(0 != len)
			++ret;
		// move forward
		s += len;
	}
	while( 0 != len && not_empty(s) );
	return ret;
}

/// Parses space separated list of unsigned integers
#ifdef IO_HAS_CONNCEPTS
template<typename T>
	requires( io::is_unsigned_integer_v<T> )
static util::intrusive_array<T> parse_string_array(const io::const_string& val) noexcept
#else
template<typename T>
static util::intrusive_array<T> parse_string_array(
			const io::const_string& val,
			typename std::enable_if<
				io::is_unsigned_integer<T>::value
			>::type* = nullptr
		) noexcept
#endif // IO_HAS_CONNCEPTS
{

	std::size_t size = count_string_numbers(val);
	util::intrusive_array<T> ret;
	// Parse numbers if any
	if( io_likely(0 != size) ) {
		ret = util::intrusive_array<T>( size );
		const char* s = val.data();
		for(std::size_t i=0; i < size; i++) {
			ret[i] = static_cast<T>( next_uint(s, s) );
		}
		return ret;
	}
	return ret;
}

static float parse_float(const io::const_string& val)
{
	float ret;
	io::from_string(val,ret);
	return ret;
}

static void parse_vec4(const io::const_string& val, float * ret)
{
	const char* s = val.data();
	for( std::size_t i = 0; i < 4; i++) {
		ret[i] = next_float(s, s);
	}
}

static io::xml::s_event_stream_parser open_parser(io::s_read_channel&& src)
{
	std::error_code ec;
	io::xml::s_event_stream_parser ret = io::xml::event_stream_parser::open(ec, std::move(src) );
	io::check_error_code( ec );
	return ret;
}

static io::const_string link_unref(const io::const_string& ref) noexcept
{
	return io::const_string( ref.data()+1, ref.length()-1 );
}

// parser

bool parser::is_element(const io::xml::start_element_event& e,const io::const_string& ln) noexcept
{
	return e.name().local_name() == ln;
}

bool parser::is_element(const io::xml::end_element_event& e,const io::const_string& ln) noexcept
{
	return e.name().local_name() == ln;
}

parser::parser(io::s_read_channel&& src) noexcept:
	xp_( open_parser( std::forward<io::s_read_channel>(src) ) ),
	library_materials_( xp_->precache("library_materials") ),
	library_effects_( xp_->precache("library_effects") ),
	library_geometries_( xp_->precache("library_geometries") ),
	library_visual_scenes_( xp_->precache("library_visual_scenes") ),
	library_images_( xp_->precache("library_images") )
{}

io::const_string parser::get_attr(const io::xml::start_element_event& sev, const char* name)
{
	auto ret = sev.get_attr("",name);
	if( io_unlikely( !ret  ) ) {
		std::string emsg("COLLADA error [");
		emsg.append( std::to_string( xp_->row() ) );
		emsg.push_back(',');
		emsg.append( std::to_string( xp_->col() ) );
		emsg.append("] ");
		emsg.append(sev.name().local_name().data() );
		emsg.push_back(' ');
		emsg.append( name );
		emsg.append(" attribute is mandatory");
		throw std::runtime_error( emsg );
	}
	return *ret;
}

// XML parsing functions

io::xml::state_type parser::to_next_state() noexcept
{
	io::xml::state_type ret;
	bool parsing = true;
	// Scan for next XML state
	do {
		ret = xp_->scan_next();
		switch(ret) {
		case io::xml::state_type::dtd:
			xp_->skip_dtd();
			break;
		case io::xml::state_type::comment:
			xp_->skip_comment();
			break;
		default:
			parsing = false;
			break;
		}
	} while(parsing);
	return ret;
}

inline void parser::check_eod( io::xml::state_type state,const char* msg)
{
	if(io::xml::state_type::eod == state) {
		if( xp_->is_error() )
			throw_parse_error();
		else
			throw std::logic_error( msg );
	}
}


bool parser::is_end_element(io::xml::event_type et,const io::const_string& local_name)
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


io::xml::event_type parser::to_next_event(io::xml::state_type& state)
{
	io::xml::event_type ret = io::xml::event_type::start_document;
	bool parsing = true;
	do {
		state = to_next_state();
		switch(state) {
		case io::xml::state_type::eod:
		case io::xml::state_type::event:
			parsing = false;
			break;
		case io::xml::state_type::cdata:
			xp_->read_cdata();
			break;
		case io::xml::state_type::characters:
			xp_->skip_chars();
			break;
		case io::xml::state_type::comment:
			xp_->skip_comment();
			break;
		case io::xml::state_type::dtd:
			xp_->skip_dtd();
			break;
		case io::xml::state_type::initial:
			break;
		}
	}
	while( parsing );
	return ret;
}

io::xml::event_type parser::to_next_tag_event(io::xml::state_type& state)
{
	io::xml::event_type ret = io::xml::event_type::start_document;
	bool parsing = true;
	do {
		ret = to_next_event(state);
		if( io::xml::state_type::eod == state )
			break;
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
			break;
		}
	}
	while( parsing );
	return ret;
}

io::xml::start_element_event parser::to_next_tag_start(io::xml::state_type& state)
{
	io::xml::event_type et;
	do {
		et = to_next_tag_event(state);
		if(io::xml::event_type::end_element == et)
			xp_->parse_end_element();
		else if(io::xml::state_type::eod == state)
			return io::xml::start_element_event();
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

// effect library

s_texture parser::parse_effect_texture_ref(const io::xml::start_element_event& sev)
{
	io::const_string name = get_attr(sev,"texture");
	auto texcoord_attr = sev.get_attribute("","texcoord");
	return s_texture( new texture(std::move(name), std::move(texcoord_attr.first) ) );
}

void parser::parse_effect(io::const_string&& id, s_effect_library& efl)
{
	static constexpr const char* ERR_MSG = "effect is unbalanced";
	io::xml::state_type state;
	io::xml::event_type et;
	io::xml::start_element_event sev;
	effect ef;
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
			else if( is_element(sev,"newparam") ) {
				parse_new_param( get_attr(sev,"sid"), efl );
			}
			else if( is_element(sev,"constant") ) {
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
				parse_vec4( get_tag_value(), ef.value.pong.adse.vec.ambient );
			}
			else if( is_element(sev,"diffuse") ) {
				sev = to_next_tag_start(state);
				check_eod(state,ERR_MSG);
				// this is diffuse texture and not a material
				if( is_element(sev, "texture") ) {
					ef.shade = shade_type::diffuse_texture;
					ef.diffuse_tex = parse_effect_texture_ref(sev);
				}
				else {
					// this is material values
					parse_vec4( get_tag_value(), ef.value.pong.adse.vec.diffuse );
				}
			}
			else if( is_element(sev,"emission") ) {
				sev = to_next_tag_start(state);
				check_eod(state,ERR_MSG);
				parse_vec4( get_tag_value(), ef.value.pong.adse.vec.emission );
			}
			else if( is_element(sev,"specular") ) {
				sev = to_next_tag_start(state);
				check_eod(state,ERR_MSG);
				parse_vec4( get_tag_value(), ef.value.pong.adse.vec.specular );
			}
			else if( is_element(sev,"shininess") ) {
				sev = to_next_tag_start(state);
				check_eod(state,ERR_MSG);
				ef.value.pong.shininess = parse_float( get_tag_value() );
			}
			else if( is_element(sev,"index_of_refraction") ) {
				sev = to_next_tag_start(state);
				check_eod(state,ERR_MSG);
				ef.value.pong.refraction_index = parse_float( get_tag_value() );
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
				io::const_string opaque = get_attr(sev,"opaque");
				ef.value.transparent.rbg = opaque.equal("RGB_ZERO") || opaque.equal("RGB_ONE");
				ef.value.transparent.invert = opaque.equal("RGB_ZERO") || opaque.equal("A_ZERO");
				sev = to_next_tag_start(state);
				check_eod(state,ERR_MSG);
				parse_vec4( get_tag_value(), ef.value.transparent.color);
			}
			else if( is_element(sev,"bump") ) {
				sev = to_next_tag_start(state);
				check_eod(state,ERR_MSG);
				// this is bump (normal) mapping effect
				if( is_element(sev, "texture") ) {
					ef.shade = shade_type::bump_mapping;
					ef.bumpmap_tex = parse_effect_texture_ref(sev);
				}
			}
		}
	}
	while( !is_end_element(et,"effect") );
	efl->add_effect( std::forward<io::const_string>(id), std::move(ef) );
}

static surface_type sampler_by_name(const io::const_string& type) noexcept
{
	const char* val = type.data();
	surface_type ret = surface_type::untyped;
	switch( val[0] ) {
	case '1': case '2': case '3':
		if('D' == val[1])
			ret = static_cast<surface_type>( uint8_t(val[0] - '0') );
		break;
	case 'C':
		if( type.equal("CUBE") ) {
			ret = surface_type::cube;
		}
		break;
	case 'D':
		if( type.equal("DEPTH") ) {
			ret = surface_type::depth;
		}
		break;
	case 'R':
		if( type.equal("RECT") ) {
			ret = surface_type::rect;
		}
		break;
	}
	return ret;
}

void parser::parse_new_param(io::const_string&& sid,s_effect_library& efl)
{
	static constexpr const char* ERR_MSG = "newparam is unbalanced";

	io::xml::state_type state;
	io::xml::event_type et;
	io::xml::start_element_event sev;
	do {
		et = to_next_tag_event(state);
		check_eod(state, ERR_MSG);
		if( is_start_element(et) ) {
			sev = xp_->parse_start_element();
			check_eod(state, ERR_MSG);
			if( is_element(sev,"surface") ) {
				surface sf;
				sf.type = sampler_by_name( get_attr(sev,"type") );
				et = to_next_tag_event(state);
				check_eod(state, ERR_MSG);
				sev = xp_->parse_start_element();
				check_eod(state, ERR_MSG);
				if( is_element(sev,"init_from") )
					sf.init_from = get_tag_value();
				efl->add_surface( std::forward<io::const_string>(sid), std::move(sf) );
			}
			else if( is_sampler_ref(sev) ) {
				et = to_next_tag_event(state);
				check_eod(state, ERR_MSG);
				sev = xp_->parse_start_element();
				check_eod(state, ERR_MSG);
				if( is_element(sev,"source") ) {
					efl->add_sampler_ref(
						std::forward<io::const_string>(sid),
						get_tag_value() );
				}
			}
		}
	}
	while( !is_end_element(et,"newparam") );
}

void parser::parse_effect_library(s_model& md)
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
			if( is_element(sev,"effect") ) {
				s_effect_library elf = md->effects();
				parse_effect(  get_attr(sev,"id"), elf  );
			}
		}
	}
	while( !is_end_element(et,library_effects_) );
}

// Materials library

void parser::parse_library_materials(s_model& md)
{
	static constexpr const char* ERR_MSG = "library_materials is unbalanced";
	io::xml::state_type state;
	io::xml::event_type et;
	io::xml::start_element_event sev;
	io::const_string material_id;
	do {
		et = to_next_tag_event(state);
		check_eod(state, ERR_MSG);
		if( is_start_element(et) ) {
			sev = xp_->parse_start_element();
			check_eod(state, ERR_MSG);
			if( is_element(sev,"material") ) {
				material_id = get_attr(sev,"id");
			}
			else if( is_element(sev,"instance_effect") ) {
				md->add_material_effect_link( std::move(material_id), link_unref(get_attr(sev,"url")) );
			}
		}
	}
	while( !is_end_element(et,library_materials_) );
}

// Geometry

input parser::parse_input(const io::xml::start_element_event& e)
{
	input ret;
	io::const_string sematic = get_attr(e,"semantic");
	constexpr const char* NAMES[7] = {
		"VERTEX",
		"POSITION",
		"NORMAL",
		"TEXCOORD",
		"COLOR",
		"TANGENT",
		"BITANGENT"
	};
	for(uint8_t i=0; i < 7; i++) {
		if( sematic.equal( NAMES[i] ) ) {
			ret.type = static_cast<semantic_type>(i);
			break;
		}
	}
	// remove # at the begin
	ret.accessor_id = link_unref(get_attr(e,"source"));

	//
	ret.offset = parse_sizet( get_attr(e,"offset") );
	// optional attributes
	auto attr = e.get_attribute("","set");
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
			if( is_element(sev,"param") ) {
				parameter p;
				auto attr = sev.get_attribute("","name");
				if(attr.second)
					p.name = attr.first;
				io::const_string type = get_attr(sev,"type");
				p.presision = type.equal("float") ? presision_type::float32_t : presision_type::double64_t;
				acsr->add_parameter( std::move(p) );
			}
		}
	}
	while( !is_end_element(et,"accessor") );
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
			if( is_element(sev, "technique_common") ) {
				continue;
			}
			else if( is_element(sev,"accessor") ) {
				io::const_string id = link_unref( get_attr(sev,"source") );
				auto attr = sev.get_attribute("","count");
				std::size_t count = attr.second ? parse_sizet( attr.first ) : 0;
				attr = sev.get_attribute("","stride");
				std::size_t stride = attr.second ? parse_sizet( attr.first ) : 0;
				s_accessor acsr( new accessor( std::move(id), count, stride ) );
				parse_accessor(acsr);
				src->add_accessor( std::move(acsr) );
			}
			else if( is_element(sev, "float_array") ) {
				io::const_string id = get_attr(sev,"id");
				std::size_t data_size = parse_sizet( get_attr(sev,"count").data() );

				io::const_string data_str = get_tag_value();
				float_array data(data_size);
				std::size_t actual_size = parse_string_array( data_str, const_cast<float*>( data.get() ), data_size);
				if( io_likely(actual_size == data_size ) ) {
					src->add_float_array( std::move(id), std::move(data) );
				}
				else {
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
	while( !is_end_element(et, "source") );
}


static constexpr const char* PRIMITIVES[7] = {
	"lines",
	"linestrips",
	"polygons",
	"polylist",
	"triangles",
	"trifans",
	"tristrips"
};

bool parser::is_sub_mesh(const io::xml::start_element_event& sev) noexcept
{
	for(uint8_t i=0; i < 7; i++) {
		if( is_element(sev,PRIMITIVES[i]) )
			return true;
	}
	return false;
}

bool parser::is_sampler_ref(const io::xml::start_element_event& sev) noexcept
{
	const char * ln = sev.name().local_name().data();
	// ignore 1D, 2D, 3D, CUBE, DEPTH, RECT
	// should be taken from argument
	return 0 == io_memcmp(ln, "sampler", 6);
}

s_sub_mesh parser::parse_sub_mesh(const io::const_string& type, io::const_string&& mat, std::size_t count)
{
	primitive_type pt = primitive_type::triangles;
	for(uint8_t i=0; i < 7; i++) {
		if( type.equal( PRIMITIVES[i] ) ) {
			pt = static_cast<primitive_type>(i);
			break;
		}
	}
	std::string err_msg( type.data() );
	err_msg.append(" is unbalanced");
	const char* ERR_MSG = err_msg.data();

	sub_mesh::input_library_t layout;
	unsigned_int_array idx;
	byte_array vcount;

	io::xml::state_type state;
	io::xml::event_type et;
	io::xml::start_element_event sev;
	do {
		et = to_next_tag_event(state);
		check_eod(state, ERR_MSG);
		if( is_start_element(et) ) {
			sev = xp_->parse_start_element();
			check_eod(state, ERR_MSG);
			if( is_element(sev,"input") ) {
				layout.emplace_back( parse_input(sev) );
			} else if( is_element(sev,"vcount") ) {
				vcount = parse_string_array<uint8_t>( get_tag_value() );
			} else if( is_element(sev,"p") ) {
				idx = parse_string_array<unsigned int>( get_tag_value() );
			}
		}
	}
	while( !is_end_element(et, type) );

	layout.shrink_to_fit();

	return s_sub_mesh(
			   new sub_mesh(
				   pt,
				   std::forward<io::const_string>(mat),
				   count,
				   std::move(layout),
				   std::move(idx),
				   std::move(vcount)
			   )
		   );
}

void parser::parse_mesh(const s_mesh& m)
{
	static constexpr const char* ERR_MSG = "mesh is unbalanced";

	io::xml::state_type state;
	io::xml::event_type et;
	io::xml::start_element_event sev;

	do {
		et = to_next_tag_event(state);
		check_eod(state, ERR_MSG);
		if( is_start_element(et) ) {
			sev = xp_->parse_start_element();
			check_eod(state, ERR_MSG);
			if( is_element(sev, "source") ) {
				io::const_string id = get_attr(sev,"id");
				s_source src( new source() );
				parse_source( src );
				m->add_source( std::move(id), std::move(src) );
			}
			else if( is_element(sev, "vertices") ) {
				et = to_next_tag_event(state);
				check_eod(state, ERR_MSG);
				sev = xp_->parse_start_element();
				check_eod(state, ERR_MSG);
				if( is_element(sev, "input") )
					m->set_pos_src_id( link_unref( get_attr(sev,"source") ) );
			}
			else if( is_sub_mesh(sev) ) {
				auto mat = sev.get_attribute("","material");
				io::const_string name = sev.name().local_name();
				std::size_t count = parse_sizet( get_attr(sev,"count") );
				m->add_sub_mesh( parse_sub_mesh(name, std::move(mat.first), count) );
			}
		}
	}
	while( !is_end_element(et, "mesh") );
}

void parser::pase_geometry_library(s_model& md)
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
			else if( is_element(sev,"geometry") ) {
				geometry_id = get_attr(sev,"id");
				auto attr = sev.get_attribute("","name");
				geometry_name = attr.second ? attr.first : geometry_id;
			}
			else if( is_element(sev,"mesh") ) {
				s_mesh m( new mesh(std::move(geometry_name) ) );
				parse_mesh(m);
				md->add_geometry( std::move(geometry_id), std::move(m) );
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
				nd.geo_ref.url = link_unref( get_attr(sev,"url") );
				auto attr = sev.get_attribute("","name");
				if( attr.second )
					nd.geo_ref.name.swap( attr.first );
			}
			else if(is_element(sev,"instance_material") ) {
				nd.geo_ref.mat_ref.target = link_unref( get_attr(sev,"target") );
				nd.geo_ref.mat_ref.symbol = get_attr(sev,"symbol");
			}
		}
	}
	while( !is_end_element(et, "node") );
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
			if( is_element(sev, "node") ) {
				node nd;
				nd.id = get_attr(sev, "id");
				auto attr = sev.get_attribute("","name");
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
	while( !is_end_element(et, "visual_scene") );
}

void parser::parse_library_images(s_model& md)
{
	static const char* ERR_MSG = "library_image is unbalanced";
	io::xml::state_type state;
	io::xml::event_type et;
	io::xml::start_element_event sev;
	do {
		et = to_next_tag_event(state);
		check_eod(state, ERR_MSG);
		if( is_start_element(et) ) {
			sev = xp_->parse_start_element();
			check_eod(state, ERR_MSG);
			if( is_element(sev, "image") ) {
				io::const_string id = get_attr(sev, "id");
				sev = to_next_tag_start(state);
				check_eod(state, ERR_MSG);
				// TODO: add next expected
				if( !is_element(sev,"init_from") )
					throw std::runtime_error("init_from element expected");
				md->add_image( std::move(id), get_tag_value() );
			}
		}
	}
	while( !is_end_element(et, library_images_) );
}

void parser::library_visual_scenes(s_model& md)
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
			if( is_element(sev,"visual_scene") ) {
				// name attribute is optional
				s_scene scn( new scene( get_attr(sev,"id"), sev.get_attribute("","name").first ) );
				parse_visual_scene(scn);
				md->set_scene( std::move(scn) );
			}
		}
	}
	while( !is_end_element(et,library_visual_scenes_) );
}


s_model parser::load()
{
	s_model ret( new model() );
	io::xml::state_type state;
	io::xml::start_element_event e = to_next_tag_start(state);
	check_eod(state, "Expecting COLLADA model file");
	if( ! is_element(e,"COLLADA") )
		throw std::runtime_error("Expecting COLLADA model file");
	do {
		e = to_next_tag_start(state);
		// nothing to do, this is some empty section
		if( e.empty_element() )
			continue;
		else if( is_element(e,library_images_) ) {
			parse_library_images(ret);
		}
		else if( is_element(e,library_materials_) ) {
			parse_library_materials(ret);
		}
		else if( is_element(e,library_effects_) ) {
			parse_effect_library(ret);
		}
		else if( is_element(e,library_geometries_) ) {
			pase_geometry_library(ret);
		}
		else if( is_element(e,library_visual_scenes_) ) {
			library_visual_scenes(ret);
		}
		// skip sections we are not interesting in
	}
	while( state != io::xml::state_type::eod );

	if( xp_->is_error() )
		throw_parse_error();

	return ret;
}

void parser::throw_parse_error()
{
	std::error_code ec;
	xp_->get_last_error(ec);
	std::string msg("XML error [");
	msg.append( std::to_string( xp_->row() ) );
	msg.push_back(',');
	msg.append( std::to_string( xp_->col() ) );
	msg.append("] ");
	msg.append( ec.message() );
	throw std::runtime_error( msg );
}

parser::~parser() noexcept
{}

} // namespace collada
