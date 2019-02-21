#ifndef __PARSER_HPP_INCLUDED__
#define __PARSER_HPP_INCLUDED__

#include <object.hpp>
#include <scoped_array.hpp>
#include <xml_reader.hpp>
#include <xml_lexcast.hpp>

#include <renderer.hpp>

#include <list>

namespace collada {


struct image
{
	io::const_string id;
	io::const_string init_from;
};

struct phong_effect
{
	float ambient[4];
	float emission[4];
	float diffuse[4];
	float specular[4];
	float shininess;
	float refraction_index;
};

struct material
{
	io::const_string id;
	phong_effect effect;
};

struct float_array
{
	io::const_string id;
	io::const_string name;
	io::scoped_arr<float> data;
};

struct source {
	io::const_string name;
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

class const_string_hasher: public std::unary_function<std::size_t,io::const_string>
{
public:
	inline std::size_t operator()(const io::const_string& str) const noexcept
	{
		return str.hash();
	}
};

typedef std::unordered_map<
					io::const_string, source,
					const_string_hasher, std::equal_to<io::const_string>,
					std::allocator<std::pair<const io::const_string,source> >
				>  source_container;


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
	source_container sources;
	primitive_type type;
	std::vector<input> inputs;
	std::vector<unsigned int> indecises;
};

struct model
{
	std::vector<image> images;
	std::vector<material> materials;
	std::vector<mesh> meshes;
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
		void check_eod(io::xml::state_type state,const std::string& msg);

		// FIXME remove this
		io::xml::start_element_event skip_to_tag(const char *local_name);

		/// Skip the element including all child elements
		void skip_element(const io::xml::start_element_event& e);

		/// drop parser to next tag start event
		/// returns current tag value
        io::const_string get_tag_value();

		// parse specific tags

		void parse_pong(phong_effect& effect);

		std::vector<material> read_effect_library();

	private:
		io::xml::s_event_stream_parser xp_;
		std::error_code ec_;
};

} // namespace collada

#endif // __PARSER_HPP_INCLUDED__
