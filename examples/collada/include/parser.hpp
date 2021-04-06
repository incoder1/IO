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
#ifndef __PARSER_HPP_INCLUDED__
#define __PARSER_HPP_INCLUDED__

#include <stdexcept>
#include <string>

#include "collada.hpp"

#include <xml_parse.hpp>

namespace collada {


/// Parses COLLADA model assets from DAE XML stream
class parser {
public:
	/// Constructs COLLADA parser for read stream points to XML data
	/// \param src XML source data
	parser(io::s_read_channel&& src) noexcept;


	/// Parse COLLADA XML into internal memory structures
	/// which can be used by model loader, to load by renderer engine
	s_model load();

	virtual ~parser() noexcept;

private:

	// StAX parsing helpers
	/// Move XML parser to next parser state, i.e. skip XML specific declarations like DTD etc.
	io::xml::state_type to_next_state() noexcept;
	/// Move XML parser next parser event, i.e. skip spaces between tags, comments, DTD etc.
	io::xml::event_type to_next_event(io::xml::state_type& state);
	/// Move XML parser to next tag event, i.e. element start or element end
	/// skips any comments or characters
	io::xml::event_type to_next_tag_event(io::xml::state_type& state);
	/// Move XML parser to next element start event, skis anything before this event
	io::xml::start_element_event to_next_tag_start(io::xml::state_type& state);
	/// Checks for end of document, i.e. no more data or parsing error
	inline void check_eod(io::xml::state_type state,const char* msg);
	/// Checks XML parser event is end element event of the required element local name
	bool is_end_element(io::xml::event_type et,const io::const_string& local_name);
	/// Checks XML parser event is end element event of the required full tag name
	inline bool is_end_element(io::xml::event_type et,const io::xml::qname& tagname);

	/// Checks start element points to element of the required element local name
	static bool is_element(const io::xml::start_element_event& e,const io::const_string& ln) noexcept;
	/// Checks end element points to element of the required element local name
	static bool is_element(const io::xml::end_element_event& e,const io::const_string& ln) noexcept;

	bool is_element(const io::xml::start_element_event& e,const char* s) noexcept
	{
		// do not, pre-cache sso optimized strings, since it have no sense
		return  e.name().local_name().sso()
		?  e.name().local_name().equal(s)
		: is_element(e, xp_->precache(s) );
	}

	bool is_element(const io::xml::end_element_event& e,const char* s) noexcept
	{
		// do not, pre-cache sso optimized strings, since it have no sense
		return  e.name().local_name().sso()
		?  e.name().local_name().equal(s)
		: is_element(e, xp_->precache(s) );
	}

	/// Throw a parsing error, with line and column number
	void throw_parse_error();

	/// Check to points/lines/triangles/polyline etc
	bool is_sub_mesh(const io::xml::start_element_event& sev) noexcept;
	/// Check for reference on texture sampler element
	bool is_sampler_ref(const io::xml::start_element_event& sev) noexcept;

	/// get's mandatory tag attribute, or throw error if no such attribute
	io::const_string get_attr(const io::xml::start_element_event& sev, const char* name);

	/// returns current tag value
	io::const_string get_tag_value();

	// parse effect library functions
	s_texture parse_effect_texture_ref(const io::xml::start_element_event& sev);
	void parse_effect(io::const_string&& id, s_effect_library& efl);
	void parse_new_param(io::const_string&& sid, s_effect_library& efl);
	void parse_effect_library(s_model& md);

	// parse materials library functions
	input parse_input(const io::xml::start_element_event& e);
	void parse_library_materials(s_model& md);

	// parse geometry library functions
	unsigned_int_array parse_index_data();
	s_sub_mesh parse_sub_mesh(const io::const_string& type, io::const_string&& mat, std::size_t count);

	void parse_accessor(s_accessor& src);
	void parse_source(const s_source& src);

	void parse_mesh(const s_mesh& m);
	void pase_geometry_library(s_model& md);

	void parse_node(node& nd);
	void parse_visual_scene(s_scene& scn);
	void library_visual_scenes(s_model& md);

	// Parse image library functions
	void parse_library_images(s_model& md);


private:
	io::xml::s_event_stream_parser xp_;
// pre-cache root section elements, to speed up the main parsing loop
	io::const_string library_materials_;
	io::const_string library_effects_;
	io::const_string library_geometries_;
	io::const_string library_visual_scenes_;
	io::const_string library_images_;
};

} // namespace collada

#endif // __PARSER_HPP_INCLUDED__
