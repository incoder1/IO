#ifndef __PARSER_HPP_INCLUDED__
#define __PARSER_HPP_INCLUDED__

#include <object.hpp>
#include <scoped_array.hpp>
#include <xml_reader.hpp>
#include <xml_lexcast.hpp>

#include <renderer.hpp>

namespace collada {


struct image
{
	io::const_string id;
	io::const_string init_from;
};

struct float_array
{
	io::const_string id;
	io::const_string name;
	io::scoped_arr<float> data;
};

struct source {
	uint8_t stride;
	float_array data;
};


struct input
{
	io::const_string semantic;
	io::const_string source;
	uint8_t offset;
	uint8_t set;
};

struct polylist
{
	std::vector<input> inputs;
	std::vector<unsigned int> indecises;
};

struct mesh
{
	std::unordered_map<std::size_t, source> sources;
};

struct geometry
{
	std::vector<mesh> meshes;
};

class parser final: io::object
{
	public:
		parser(io::s_read_channel&& src) noexcept;
		virtual ~parser() noexcept;
	private:
		io::unsafe<io::xml::reader> rd_;
};

} // namespace collada

#endif // __PARSER_HPP_INCLUDED__
