#ifndef __PARSER_HPP_INCLUDED__
#define __PARSER_HPP_INCLUDED__

#include <object.hpp>
#include <scoped_array.hpp>
#include <xml_reader.hpp>
#include <xml_lexcast.hpp>

namespace collada {


enum class up_direction_type {
	up_x,up_y,up_z
};


struct float_array
{
	io::const_string id;
	io::const_string name;
	io::scoped_arr<float> data;
};


struct asset_info {
	float unit_size;
	up_direction_type up_direction;
};

struct convex_mesh
{
};

struct mesh {
	io::scoped_arr<unsigned int> index;
	io::scoped_arr<float> vertex;
};


struct geometry
{
};


class parser final: io::object
{
	public:
		parser(const io::s_read_channel& src);
		virtual ~parser() noexcept;
};

} // namespace collada

#endif // __PARSER_HPP_INCLUDED__
