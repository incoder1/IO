#ifndef __PARSER_HPP_INCLUDED__
#define __PARSER_HPP_INCLUDED__

#include <stdexcept>
#include <string>

#include "collada.hpp"

#include <xml_parse.hpp>
#include <xml_lexcast.hpp>

namespace collada {


class parser {
public:
	parser(io::s_read_channel&& src) noexcept;

	s_model load();

	virtual ~parser() noexcept;

private:

	// StAX parsing helpers
	io::xml::state_type to_next_state() noexcept;
	io::xml::event_type to_next_event(io::xml::state_type& state);
	io::xml::event_type to_next_tag_event(io::xml::state_type& state);
	io::xml::start_element_event to_next_tag_start(io::xml::state_type& state);
	inline void check_eod(io::xml::state_type state,const char* msg);
	bool is_end_element(io::xml::event_type et,const io::cached_string& local_name);
	inline bool is_end_element(io::xml::event_type et,const io::xml::qname& tagname);


	static bool is_element(const io::xml::start_element_event& e,const io::cached_string& ln) noexcept;
	static bool is_element(const io::xml::end_element_event& e,const io::cached_string& ln) noexcept;


	bool is_element(const io::xml::start_element_event& e,const char* s) noexcept
	{
		return  e.name().local_name().sso()
		?  e.name().local_name().equal(s)
		: is_element(e, xp_->precache(s) );
	}

	bool is_element(const io::xml::end_element_event& e,const char* s) noexcept
	{
		return  e.name().local_name().sso()
		?  e.name().local_name().equal(s)
		: is_element(e, xp_->precache(s) );
	}

	bool is_index_data(const io::xml::start_element_event& sev,primitive_type& pt) noexcept;


	/// Skip the element including all child elements
	void skip_element(const io::xml::start_element_event& e);

	/// returns current tag value
	io::const_string get_tag_value();

	// parse effect library functions
	void parse_effect(effect& effect);
	void parse_effect_library(s_model& md);

	// parse materials library functions
	void parse_library_materials(s_model& md);

	// geometry
	void parse_vertex_data(const s_mesh& m);
	void parse_index_data(index_data& index);

	void parse_accessor(s_accessor& src);
	void parse_source(const s_source& src);

	void parse_mesh(const s_mesh& m);
	void pase_geometry_library(s_model& md);

	void parse_node(node& nd);
	void parse_visual_scene(s_scene& scn);
	void library_visual_scenes(s_model& md);

	// images
	void parse_library_images(s_model& md);

	void throw_parse_error();

private:
	io::xml::s_event_stream_parser xp_;
	// pre-cached rare element names
#define CACHE_STR(__x) io::cached_string __x##_
// pre-cache needed elements, to speed up the main parsing loop
// using pointers compare instead of a string compare;
	CACHE_STR(library_materials);
	CACHE_STR(library_effects);
	CACHE_STR(library_geometries);
	CACHE_STR(library_visual_scenes);
	CACHE_STR(library_images);
#undef CACHE_STR
};

} // namespace collada

#endif // __PARSER_HPP_INCLUDED__
