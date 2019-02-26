#ifndef __PARSER_HPP_INCLUDED__
#define __PARSER_HPP_INCLUDED__

#include "collada.hpp"

#include <xml_reader.hpp>
#include <xml_lexcast.hpp>

namespace collada {


class parser {
public:
	parser(io::s_read_channel&& src) noexcept;

	model load();

	virtual ~parser() noexcept;

private:

	// StAX parsing helpers
	io::xml::state_type to_next_state();
	io::xml::event_type to_next_tag_event(io::xml::state_type& state);
	io::xml::start_element_event to_next_tag_start(io::xml::state_type& state);
	inline void check_eod(io::xml::state_type state,const char* msg);
	inline void check_eod(io::xml::state_type state,const std::string& msg);
	bool is_end_element(io::xml::event_type et,const io::cached_string& local_name);

	template<class T>
	static bool is_element(const T& e,const io::cached_string& local_name) noexcept {
		// check with cache miss
		return  e.name().local_name() == local_name;
	}

	template<class T>
	bool is_element(const T& e, const char* local_name) {
		return is_element( e, xp_->precache(local_name) );
	}

	/// Skip the element including all child elements
	void skip_element(const io::xml::start_element_event& e);

	/// drop parser to next tag start event
	/// returns current tag value
	io::const_string get_tag_value();

	// parse effect library functions
	void parse_effect(effect& effect);
	void parse_effect_library(model& md);

	// geometry
	void parse_vertex_data(mesh& m);
	void parse_index_data(mesh& m);

	void parse_accessor(accessor& src);
	void parse_source(source& src);

	void parse_mesh(mesh& m);
	void pase_geometry_library(model& md);

private:
	io::xml::s_event_stream_parser xp_;
	std::error_code ec_;
	// pre-cached rare element names
#define CACHE_STR(__x) io::cached_string __x##_
// pre-cache needed elements, to speed up the main parsing loop
// using pointers compare instead of a string compare
	CACHE_STR(asset);
	CACHE_STR(accessor);
	CACHE_STR(effect);
	CACHE_STR(float_array);
	CACHE_STR(geometry);
	CACHE_STR(mesh);
	CACHE_STR(vertices);
	CACHE_STR(source);
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
};

} // namespace collada

#endif // __PARSER_HPP_INCLUDED__
