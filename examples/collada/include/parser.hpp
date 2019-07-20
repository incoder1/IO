#ifndef __PARSER_HPP_INCLUDED__
#define __PARSER_HPP_INCLUDED__

#include <stdexcept>
#include <string>

#include "collada.hpp"

#include <xml_parse.hpp>
#include <xml_lexcast.hpp>

namespace collada {


/// Parses COLLADA model asssets from XML file
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
	/// Move XML parser to next parser state, i.e. skip XML specific declarations like dtd etc
	io::xml::state_type to_next_state() noexcept;
	/// Move XML parser next parser event, i.e. skip spaces between tags, comments, dtd
	io::xml::event_type to_next_event(io::xml::state_type& state);
	/// Move XML parser to next tag event, i.e. element start or element end
	/// skips any comments or characters
	io::xml::event_type to_next_tag_event(io::xml::state_type& state);
	/// Move XML parser to next element start event, skis anything before this event
	io::xml::start_element_event to_next_tag_start(io::xml::state_type& state);
	/// Checks for end of document, i.e. no more data or parsing error
	inline void check_eod(io::xml::state_type state,const char* msg);
	/// Checks XML parser event is end element event of the required element local name
	bool is_end_element(io::xml::event_type et,const io::cached_string& local_name);
	/// Checks XML parser event is end element event of the required full tag name
	inline bool is_end_element(io::xml::event_type et,const io::xml::qname& tagname);

	/// Checks start element points to element of the required element local name
	static bool is_element(const io::xml::start_element_event& e,const io::cached_string& ln) noexcept;
	/// Checks end element points to element of the required element local name
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

	/// Check to points/lines/triangles/polyline etc
	bool is_sub_mesh(const io::xml::start_element_event& sev) noexcept;
	/// Check for reference on texture sampler element
	bool is_sampler_ref(const io::xml::start_element_event& sev) noexcept;

	/// get's mandatory tag attribute, or throw error if no such attribute
	io::const_string get_attr(const io::xml::start_element_event& sev, const char* name);

	/// returns current tag value
	io::const_string get_tag_value();

	// parse effect library functions
	void parse_effect(io::const_string&& id, s_effect_library& efl);
	void parse_new_param(io::const_string&& sid, s_effect_library& efl);
	void parse_effect_library(s_model& md);

	// parse materials library functions
	input parse_input(const io::xml::start_element_event& e);
	void parse_library_materials(s_model& md);

	// geometry
	unsigned_int_array parse_index_data();
	s_sub_mesh parse_sub_mesh(const io::const_string& type, io::const_string&& mat, std::size_t count);

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
