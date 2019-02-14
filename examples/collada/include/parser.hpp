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
};

struct material
{
	io::const_string name;
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



class parser final: io::object
{
	public:
		parser(io::s_read_channel&& src) noexcept;

		model load();

		virtual ~parser() noexcept;

	private:


		std::vector<material> read_materials();

	private:

		io::unsafe<io::xml::reader> rd_;
};

} // namespace collada

#endif // __PARSER_HPP_INCLUDED__
