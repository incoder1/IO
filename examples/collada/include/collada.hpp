#ifndef __COLLADA_HPP_INCLUDED__
#define __COLLADA_HPP_INCLUDED__

#include <algorithm>
#include <vector>
#include <memory>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>

#include <object.hpp>
#include <scoped_array.hpp>
#include <conststring.hpp>
#include <stringpool.hpp>

#include "intrusive_array.hpp"

namespace collada {

namespace detail {

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

#ifdef __IO_WINDOWS_BACKEND__
	typedef io::enclave_allocator< std::pair<const io::const_string, V> > hashmap_allocator;
	typedef io::enclave_allocator<V> container_allocator;
#else
	typedef io::h_allocator< std::pair<const io::const_string, V> > hashmap_allocator;
	typedef io::h_allocator<V> container_allocator;
#endif // __IO_WINDOWS_BACKEND__


public:
	typedef std::unordered_map<io::const_string,V,hash,pred,hashmap_allocator> param_library;
	typedef std::vector<V,container_allocator> param_vector;
};

} // namespace detail

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

struct phong_effect {
	float ambient[4];
	float emission[4];
	float diffuse[4];
	float specular[4];
	float shininess;
	float refraction_index;
};

struct reflectivity_effect {
	float color[4];
	float value;
};

struct transparency_effect {
	float color[4];
	bool used;
	bool rbg;
	bool invert;
};

struct ad_3dsmax_ext_effect {
	bool double_sided;
	bool wireframe;
	bool faceted;
};

struct sampler_effect {
	phong_effect mat;
	transparency_effect transparent;
	reflectivity_effect reflect;
	shade_type shade;
	ad_3dsmax_ext_effect ext_3max;
	float bump[4];
};

union effect {
	struct __value_t {
		phong_effect mat;
		transparency_effect transparent;
		reflectivity_effect reflect;
		shade_type shade;
		ad_3dsmax_ext_effect ext_3max;
	} value;
	sampler_effect text;
};

enum class primitive_type: uint8_t {
	lines = 0,
	linestrips = 1,
	polygons = 2,
	polylist = 3,
	triangles = 4,
	trifans = 5,
	tristrips = 6
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

enum class presision {
	float32_t, // float
	double64_t // double
};


struct parameter {
	io::const_string name;
	presision presision;
};

// Possible vertex layout
// Vertex  { [position]{x,y,z},[normal]{x,y,z},[color]{r,g,b,a} }
// and we have single triangle model
// Next accessors:
//  position {3, 3, { {"x",float32_t},{"y",float32_t},{"y",float32_t} } }
//  normal {4, 3, { {"x",float32_t},{"y",float32_t},{"y",float32_t} } }
//  tangent {6, 4, { {"r",float32_t},{"g",float32_t},{"b",float32_t},{"a",float32_t} }}
class accessor final:public io::object {
public:

	typedef detail::param<parameter>::param_vector layout_t;
	typedef layout_t::const_iterator const_iterator;

	accessor(io::const_string&& src_id,std::size_t count, std::size_t stride);
	virtual ~accessor() noexcept override;

	void add_parameter(parameter&& prm);

	io::const_string source_id() const noexcept {
		return source_id_;
	}

	std::size_t count() const noexcept {
		return count_;
	}

	std::size_t stride() const noexcept {
		return stride_;
	}

	std::size_t layout() const noexcept {
		return layout_.size();
	}

	const_iterator cbegin() const noexcept {
		return layout_.cbegin();
	}

	const_iterator cend() const noexcept {
		return layout_.cend();
	}

private:
	io::const_string source_id_;
	// last position in the index array
	std::size_t count_;
	// count of parameter in this vertex data semantic
	std::size_t stride_;
	// parameters layout
	layout_t layout_;
};

DECLARE_IPTR(accessor);

struct input_channel {
	// Type of the data
	semantic_type type;
	// ID of the accessor where to read the actual values from.
	io::const_string accessor_id;
	std::size_t offset;
	std::size_t set;
	constexpr input_channel() noexcept:
		type(semantic_type::vertex),
		accessor_id(),
		offset(0),
		set(0)
	{}
};

typedef intrusive_array<float> float_array;
typedef intrusive_array<unsigned int> unsigned_int_array;

class source final:public io::object {
	source(const source&) = delete;
	source& operator=(const source&) = delete;
private:
	typedef detail::param< float_array >::param_library float_array_library_t;
	typedef detail::param< io::const_string >::param_library string_array_library_t;
	typedef detail::param<s_accessor>::param_vector accessors_library_t;
public:

	typedef accessors_library_t::const_iterator const_iterator;

	source();
	virtual ~source() noexcept override;

	void add_float_array(io::const_string&& id, float_array&& arr);
	const float_array find_float_array(const io::const_string& id) const;

	void add_accessor(s_accessor&& acsr);

	const_iterator cbegin() const {
		return accessors_.cbegin();
	}

	const_iterator cend() const {
		return accessors_.cend();
	}

private:
	float_array_library_t float_arrays_;
	//string_array_library_t string_arrays_;
	accessors_library_t accessors_;
};

DECLARE_IPTR(source);

class index_data final:public io::object {
	index_data(const index_data&) = delete;
	index_data& operator=(const index_data&) = delete;
public:
	index_data() noexcept;
	virtual ~index_data() noexcept override;
	primitive_type primitives() const noexcept {
		return primitives_;
	}
	void set_primitives(primitive_type type) noexcept {
		primitives_ = type;
	}
	std::size_t count() const noexcept {
		return count_;
	}
	void set_count(std::size_t count) noexcept {
		count_ = count;
	}
	unsigned_int_array indices() const noexcept
	{
		return indices_;
	}
	void set_indices(unsigned_int_array&& idx) noexcept
	{
		indices_ = std::move(idx);
	}
private:
	primitive_type primitives_;
	std::size_t count_;
	unsigned_int_array indices_;
};

DECLARE_IPTR(index_data);

class parser;

class mesh final:public io::object {
	mesh(const mesh&) = delete;
	mesh& operator=(const mesh&) = delete;
private:
	typedef detail::param< s_source >::param_library source_library_t;
	typedef detail::param<input_channel>::param_vector input_channels_library_t;
public:
	typedef input_channels_library_t::const_iterator const_iterator;

	mesh(io::const_string&& name) noexcept;
	virtual ~mesh() noexcept override;

	io::const_string name() const noexcept
	{
        return name_;
	}

	void set_vertex_id(io::const_string&& id) noexcept {
		vertex_id_ = std::move(id);
	}

	io::const_string vertex_id() const noexcept {
		return vertex_id_;
	}

	const index_data* index() const noexcept {
		return index_.get();
	}

	void add_source(io::const_string&& id, s_source&& src);
	s_source find_souce(const io::const_string& id) const;

	void add_input_channel(input_channel&& ich);

	const_iterator cbegin() const {
		return input_channels_.cbegin();
	}

	const_iterator cend() const {
		return input_channels_.cend();
	}
private:
	friend class parser;
	s_index_data get_index() noexcept {
		return index_;
	}
private:
	io::const_string name_;
	io::const_string vertex_id_;
	source_library_t source_library_;
	input_channels_library_t input_channels_;
	s_index_data index_;
};

DECLARE_IPTR(mesh);

enum class node_type
{
	node,
	joint
};

struct instance_material
{
	io::const_string symbol;
	io::const_string target;
};

struct instance_geometry
{
    io::const_string url;
    io::const_string name;
    instance_material mat_ref;
};

struct node {
	node_type type;
	io::const_string id;
	io::const_string name;
	instance_geometry geo_ref;
};

class scene final:public io::object
{
public:
	typedef detail::param<node>::param_vector nodes_container;
	typedef nodes_container::const_iterator const_iterator;
	scene(io::const_string&& id,io::const_string&& name);
	virtual ~scene() noexcept override;
	void add_node(node&& nd);
	const_iterator cbegin() const {
		return nodes_.cbegin();
	}
	const_iterator cend() const {
		return nodes_.cend();
	}
private:
	io::const_string id_;
	io::const_string name_;
	nodes_container nodes_;
};

DECLARE_IPTR(scene);

class model final:public io::object {
	model(const model&) = delete;
	model& operator=(const model&) = delete;
private:
	typedef detail::param< std::shared_ptr<effect> >::param_library effect_library_t;
	typedef detail::param< s_mesh >::param_library geometry_library_t;
	typedef detail::param< image >::param_library image_library_t;
    // links from material library to the effect library
	typedef detail::param< io::const_string >::param_library material_library_t;
public:
	model(model&&) noexcept = default;
	model& operator=(model&&) = default;

	model();
	virtual ~model() noexcept;

	void add_effect(io::const_string&& id,effect&& e);
	std::shared_ptr<effect> find_effect(const io::const_string& id) const noexcept;

	void add_material_effect_link(io::const_string&& id,io::const_string&& effect_id);
	std::shared_ptr<effect> find_material(const io::const_string& material_id) const noexcept;

	void add_mesh(io::const_string&& id,s_mesh&& e);
	s_mesh find_mesh(const io::const_string& id) noexcept;

	s_scene scene() const noexcept {
		return scene_;
	}

	void set_scene(s_scene&& scn) noexcept {
        scene_ = std::move(scn);
	}

private:
	effect_library_t effects_;
	image_library_t images_;
	geometry_library_t meshes_;
	material_library_t materials_;
	s_scene scene_;
};

DECLARE_IPTR(model);

} // namespace collada

#endif // __COLLADA_HPP_INCLUDED__
