#ifndef __PARSER_HPP_INCLUDED__
#define __PARSER_HPP_INCLUDED__

#include <memory>
#include <vector>

#include <object.hpp>
#include <scoped_array.hpp>

#include <xml_reader.hpp>
#include <xml_lexcast.hpp>

namespace collada {


template< typename V >
class param {
private:

	class hash: public std::unary_function<std::size_t,io::const_string> {
	public:
		inline std::size_t operator()(const io::const_string& str) const noexcept {
			return str.hash();
		}
	};

	typedef std::equal_to<io::const_string> pred;

	typedef io::h_allocator< std::pair<const io::const_string, V> > allocator;

public:

	typedef std::unordered_map <io::const_string,V,hash,pred,allocator> param_library;
};

struct image {
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
	float refraction_index;
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

struct ad_3dsmax_ext
{
	bool double_sided;
	bool wireframe;
	bool faceted;
};

struct sampler_effect {
	material mat;
	transparency transparent;
	reflectivity reflect;
	shade_type shade;
	ad_3dsmax_ext ext_3max;
	float bump[4];
};

union effect {
	struct __value_t {
			material mat;
			transparency transparent;
			reflectivity reflect;
			shade_type shade;
			ad_3dsmax_ext ext_3max;
	} value;
	sampler_effect text;
};

/*
struct float_array {
	io::const_string id;
	io::const_string name;
	io::scoped_arr<float> data;
};

struct source {
	float_array data;
	uint8_t stride;
};

struct input {
	semantic_type semantic;
	io::const_string source_id;
	uint8_t offset;
	uint8_t set;
};




*/




enum class primitive_type {
	lines,
	linestrips,
	polygons,
	polylist,
	tiangles
};


enum class semantic_type {
	// special type for per-index data referring to
	// the <vertices> element carrying the per-vertex data.
    vertex,
    position,
    normal,
    texcoord,
    color,
    tangent,
    bitangent
};


struct source_data
{
    bool string_array;
    std::vector<io::const_string> strings;
    io::scoped_arr<float> values;
};

/** Accessor to a data array */
struct accessor
{
	// in number of objects
    std::size_t count;
    // size of an object, in elements (floats or strings, mostly 1)
    std::size_t size;
    // in number of values
    std::size_t offset;
    // Stride in number of values
    std::size_t stride;
    // names of the data streams in the accessors. Empty string tells to ignore.
    std::vector<io::const_string> params;
	// Suboffset inside the object for the common 4 elements. For a vector, that's XYZ, for a color RGBA and so on.
	// For example, sub_offset[0] denotes which of the values inside the object is the vector X component.
    std::size_t sub_offset[4];
	// id of the source array
	io::const_string source_id;
	// Pointer to the source array, if resolved, otherwise empty
    std::shared_ptr<source_data> data;
};

struct input_channel
{
	// Type of the data
    semantic_type type;
    // Optional index, if multiple sets of the same data type are given
    std::size_t index;
    // Offset from the indices array per the face.
    std::size_t offset;
    // ID of the accessor where to read the actual values from.
    io::const_string accessor_id;
    // Pointer to the accessor, if resolved. otherwise empty
    std::shared_ptr<accessor> resolved;
};

/// A single face in a mesh
struct face
{
    io::scoped_arr<unsigned int> indices;
};

/// Subset of a mesh with a certain material
struct sub_mesh
{
	/// subgroup identifier
    io::const_string material_id;
    /// number of faces in this sub mesh
    std::size_t faces_count;
};

struct mesh {
	primitive_type type;
	io::const_string name;
	io::const_string vertex_id;

	// Vertex data addressed by vertex indices
    std::vector<input_channel> input_channels;
    std::vector<unsigned int> face_sizes;
    std::vector<unsigned int> face_pos_indices;
	std::vector<sub_mesh> sub_meshes;

	io::scoped_arr<float> positions;
	io::scoped_arr<float> normals;
	io::scoped_arr<float> tangents;
	io::scoped_arr<float> bitangents;
	io::scoped_arr<float> text_coords;
	io::scoped_arr<float> colors;
};

class model {
	model(const model&) = delete;
	model& operator=(const model&) = delete;
private:
	typedef param< std::shared_ptr<effect> >::param_library effect_library_t;
	typedef param< std::shared_ptr<mesh> >::param_library geometry_library_t;
public:
	model(model&&) noexcept = default;
	model& operator=(model&&) = default;
	~model() noexcept = default;

	model();

	void add_effect(io::const_string&& id,effect&& e);

	std::shared_ptr<effect> find_effect(const char* id) noexcept;

	void add_mesh(io::const_string&& id,mesh&& e);

	std::shared_ptr<mesh> find_mesh(const char* id) noexcept;

private:
	effect_library_t effects_;
	std::vector<image> images_;
	geometry_library_t meshes_;
};



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

	void parse_vertex_data(mesh& m);
	void parse_index_data(mesh& m);
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
	CACHE_STR(effect);
	CACHE_STR(geometry);
	CACHE_STR(mesh);
	CACHE_STR(source);
	CACHE_STR(vertices);
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
