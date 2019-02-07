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


struct input
{
	io::const_string semantic;
	io::const_string source;
	uint8_t offset;
	uint8_t set;
};

struct poly_list
{
	std::vector<input> inputs;
	std::vector<unsigned int> indecises;
};

class cs_hash: public std::unary_function<std::size_t,io::const_string>
{
public:
	inline std::size_t operator()(const io::const_string& str) const noexcept
	{
		return str.hash();
	}
};

typedef std::unordered_map<
					io::const_string, source,
					cs_hash, std::equal_to<io::const_string>,
					std::allocator<std::pair<const io::const_string,source> >
				>  sources_container;

struct mesh
{
	sources_container sources;
	poly_list pl;
};

struct model
{
	std::vector<image> images;
	std::vector<mesh> meshes;
};

class parser final: io::object
{
	public:
		parser(io::s_read_channel&& src) noexcept;

		model load()
		{
			return model();
		}

		virtual ~parser() noexcept;
	private:

		io::unsafe<io::xml::reader> rd_;
};

} // namespace collada

#endif // __PARSER_HPP_INCLUDED__
