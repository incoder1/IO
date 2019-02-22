#ifndef __PARSER_HPP_INCLUDED__
#define __PARSER_HPP_INCLUDED__

#include <memory>

#include <object.hpp>
#include <scoped_array.hpp>
#include <xml_reader.hpp>
#include <xml_lexcast.hpp>
#include <renderer.hpp>

namespace collada {


template< typename V >
class libraries {
private:

	class hash: public std::unary_function<std::size_t,io::const_string>
	{
	public:
		inline std::size_t operator()(const io::const_string& str) const noexcept
		{
			return str.hash();
		}
	};

	typedef std::equal_to<io::const_string> pred;

	typedef io::h_allocator< std::pair<const io::const_string, V> > allocator;

public:

	typedef std::unordered_map <io::const_string,V,hash,pred,allocator> library_type;
};

struct image
{
	io::const_string id;
	io::const_string init_from;
};

enum class shade_type {
    constant,
    lambert,
    phong,
    blinn_phong
};

struct material {
	float ambient[4];
	float emission[4];
	float diffuse[4];
	float specular[4];
	float shininess;
};

struct reflectivity {
	float color[4];
	float value;
};

struct transparency {
	float color[4];
	bool used;
	bool rbg;
	bool invert;
};

struct effect
{
	material tex_mat;
	transparency transparent;
	reflectivity reflect;
	float text_bump[4];

	shade_type shade;
	float refraction_index;
	bool double_sided;
	bool wireframe;
	bool faceted;
};

struct float_array
{
	io::const_string id;
	io::const_string name;
	io::scoped_arr<float> data;
};

struct source {
	float_array data;
	uint8_t stride;
};


enum class semantic_type {
	vertex,
	normal,
	texcoord
};

struct input
{
	semantic_type semantic;
	io::const_string source_id;
	uint8_t offset;
	uint8_t set;
};


enum class primitive_type
{
	lines,
	linestrips,
	polygons,
	polylist,
	tiangles
};

struct mesh
{
	libraries<source>::library_type sources;
	primitive_type type;
	std::vector<input> inputs;
	std::vector<unsigned int> indecises;
};

class model
{
	model(const model&) = delete;
	model& operator=(const model&) = delete;
private:
	typedef libraries< std::shared_ptr<effect> >::library_type effect_library_t;
public:
	model(model&&) noexcept = default;
	model& operator=(model&&) = default;
	~model() noexcept = default;

	model():
		effects_(),
		images_(),
		meshes_()
	{}

	void add_effect(io::const_string&& id,effect&& e);

	std::shared_ptr<effect> find_effect(const char* id) noexcept;

private:
	effect_library_t effects_;
	std::vector<image> images_;
	std::vector<mesh> meshes_;
};



class parser
{
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

		// FIXME remove this
		io::xml::start_element_event skip_to_tag(const char *local_name);

		/// Skip the element including all child elements
		void skip_element(const io::xml::start_element_event& e);

		/// drop parser to next tag start event
		/// returns current tag value
        io::const_string get_tag_value();

		// parse effect library functions
		void parse_effect(effect& effect);
		void parse_effect_library(model& md);

	private:
		io::xml::s_event_stream_parser xp_;
		std::error_code ec_;
		// pre-cached rare element names
#define CACHE_STR(__x) io::cached_string __x##_
// pre-cache needed elements, to speed up the main parsing loop
// using pointers compare instead of a string compare
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
};

} // namespace collada

#endif // __PARSER_HPP_INCLUDED__
