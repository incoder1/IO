/*
 *
 * Copyright (c) 2016-2019
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
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

/// A reasonable sub-set of COLLADA model, to parse into and load into rendering engine
namespace collada {

namespace detail {

template< typename V >
class param {
private:

	typedef std::equal_to<io::const_string> pred;

#ifdef __IO_WINDOWS_BACKEND__
	typedef io::enclave_allocator< std::pair<const io::const_string, V> > hashmap_allocator;
	typedef io::enclave_allocator<V> container_allocator;
#else
	typedef io::h_allocator< std::pair<const io::const_string, V> > hashmap_allocator;
	typedef io::h_allocator<V> container_allocator;
#endif // __IO_WINDOWS_BACKEND__


public:
	typedef std::unordered_map<io::const_string,V,io::const_string_hash,pred,hashmap_allocator> param_library;
	typedef std::vector<V,container_allocator> param_vector;
};

} // namespace detail

/// Contains COLLADA model texture
class texture final: public io::object {
public:

	texture(io::const_string&& name, io::const_string&& texcoord) noexcept:
		io::object(),
		name_( std::forward<io::const_string>(name) ),
		texcoord_( std::forward<io::const_string>(texcoord) )
	{}

	virtual ~texture() override = default;

	io::const_string name() const noexcept {
		return name_;
	}

	io::const_string texcoord() const noexcept {
		return texcoord_;
	}

private:
	io::const_string name_;
	io::const_string texcoord_;
};

DECLARE_IPTR(texture);

/// COLLADA model shade type
enum class shade_type {
	/// Constant color
	constant,
	/// Lambert physical object shading type
	lambert,
	/// Phong shading aproximation type
	phong,
	/// Blin-Phong shading aproximation shading type
	blinn_phong,
	/// Phong shading aproximation with the diffuse texture base color
	diffuse_texture,
	/// Phong shading aproximation with the diffuse and bump-mapping textures
    bump_mapping
};

struct constant_effect {
	float color[4];
};

/// Any kind of Phong shading effect
struct phong_effect {
	/// Material ambient/diffuse/specular/emission components
	union __mat_adse {
		struct __adse_vectors {
			float ambient[4];
			float diffuse[4];
			float specular[4];
			float emission[4];
		} vec;
		float mat[16];
	} adse;
	/// Material shininess multiplicator
	float shininess;
	/// Material index of refraction
	float refraction_index;
};


/// Reflectivity effect components
struct reflectivity_effect {
	float color[4];
	float value;
};

/// Transparency effect components
struct transparency_effect {
	float color[4];
	bool used;
	bool rbg;
	bool invert;
};

/// Any kind of COLLADA model effect masked union
struct effect {
	union __effect_val__ {
		phong_effect pong;
		transparency_effect transparent;
		reflectivity_effect reflect;
		constant_effect constant;
	} value;
	s_texture diffuse_tex;
	s_texture bumpmap_tex;
	/// Effect shading type
	shade_type shade;
};

/// COLLADA model element types
enum class primitive_type: uint8_t {
	lines = 0,
	linestrips = 1,
	polygons = 2,
	polylist = 3,
	triangles = 4,
	trifans = 5,
	tristrips = 6
};


// special type for per-index data referring to
// the <vertices> element carrying the per-vertex data.
// i.e. vertex attribute semantic
enum class semantic_type: uint8_t {
	vertex = 0,
	position = 1,
	normal = 2,
	texcoord = 3,
	color = 4,
	tangent = 5,
	bitangent = 6
};

enum class presision_type {
	float32_t, // float
	double64_t // double
};


struct parameter {
	io::const_string name;
	presision_type presision;
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

/// COLLADA model input tag section
struct input {
	// Type of the data
	semantic_type type;
	// ID of the accessor where to read the actual values from.
	io::const_string accessor_id;
	std::size_t offset;
	std::size_t set;
	constexpr input() noexcept:
		type(semantic_type::vertex),
		accessor_id(),
		offset(0),
		set(0)
	{}
};

/// Float data array, parsed from COLLADA float_array content
/// e.g. string of space separated floats
typedef util::intrusive_array<float> float_array;
/// byte data array, parsed from COLLADA vcount content
/// e.g. string of space separated bytes
typedef util::intrusive_array<uint8_t> byte_array;
/// unsigned integers data array, parsed from COLLADA <p> tag content
/// e.g. string of space separated unsigned ints
typedef util::intrusive_array<unsigned int> unsigned_int_array;

/// COLLADA model source data section
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

/// COLLADA model geometry section, stored in geometry_library global section
class geometry:public io::object {
public:
	enum class surface_type {
		mesh,
		spline,
		convex_mesh
	};
protected:
	geometry(surface_type type,io::const_string&& name) noexcept;
public:
	virtual ~geometry() noexcept override;
	surface_type type() const noexcept {
		return type_;
	}
	io::const_string name() const noexcept {
		return name_;
	}
private:
	surface_type type_;
	io::const_string name_;
};

DECLARE_IPTR(geometry);


/// A COLLADA model mesh sub-sources, i.e. lines/tiangles/polyline etc
class sub_mesh: public io::object {
	sub_mesh(const sub_mesh&) = delete;
	sub_mesh& operator=(const sub_mesh&) = delete;
public:
	typedef detail::param<input>::param_vector input_library_t;

	sub_mesh(primitive_type type,
			io::const_string&& mat,
			std::size_t count,
			input_library_t&& layout,
			unsigned_int_array&& index,
			byte_array&& vcount) noexcept;

	virtual ~sub_mesh() noexcept override;

	/// Gests a id to the material effect in the material library
	io::const_string material() const noexcept {
		return mat_;
	}

	/// Gets layout
	input_library_t layout() const noexcept {
		return layout_;
	}

	/// Contains a list of integers that specify the vertex attributes (indices) for an individual
	/// vertex array attributes
	unsigned_int_array primitive() const noexcept {
		return primitive_;
	}

	/// Gets vertex count peer polygon for polygonal meshes
	byte_array vcount() const noexcept {
		return vcount_;
	}

	/// Gets this sub-mesh vertex count
	std::size_t count() const noexcept {
		return count_;
	}

private:
	primitive_type type_;
	io::const_string mat_;
	input_library_t layout_;
	unsigned_int_array primitive_;
	byte_array vcount_;
	std::size_t count_;
};

DECLARE_IPTR(sub_mesh);

/// COLLADA model mesh section data
class mesh final:public geometry {
	mesh(const mesh&) = delete;
	mesh& operator=(const mesh&) = delete;
private:
	typedef detail::param< s_source >::param_library source_library_t;
	typedef detail::param< s_sub_mesh >::param_vector sub_mesh_library_t;

public:
	typedef sub_mesh_library_t::const_iterator const_iterator;

	explicit mesh(io::const_string&& name) noexcept;
	virtual ~mesh() noexcept override;

	void set_pos_src_id(io::const_string&& psrcid) {
		pos_src_id_ = std::move(psrcid);
	}

	io::const_string pos_src_id() const noexcept {
		return pos_src_id_;
	}

	void add_source(io::const_string&& id, s_source&& src);
	s_source find_souce(const io::const_string& id) const;

	void add_sub_mesh(s_sub_mesh&& sm)
	{
		sub_meshes_.emplace_back( std::forward<s_sub_mesh>(sm) );
	}

	const_iterator cbegin() const noexcept {
		return sub_meshes_.cbegin();
	}

	const_iterator cend() const noexcept {
		return sub_meshes_.cend();
	}

private:
	io::const_string pos_src_id_;
	source_library_t source_library_;
	sub_mesh_library_t sub_meshes_;
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

/// COLLADA model scene section
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

/// COLLADA model surface type e.g. texture type
enum class surface_type: uint8_t {
	untyped = 0,
	sampler_1d = 1,
	sampler_2d = 2,
	sampler_3d = 3,
	cube = 4,
	depth = 5,
	rect = 6
};

/// COLLADA model surface section - i.e. texture
struct surface {
	surface_type type;
	io::const_string init_from;
};

/// COLLADA model effect_library section
class effect_library:public io::object {
	effect_library(const effect_library&) = delete;
	effect_library& operator=(const effect_library&) = delete;
private:
	typedef detail::param< std::shared_ptr<effect> >::param_library effects_lib_t;
	typedef detail::param< surface >::param_library surfaces_lib_t;
	typedef detail::param< io::const_string >::param_library sampler_refs_lib_t;
public:
	effect_library();

	void add_effect(io::const_string&& id,effect&& e);
	std::shared_ptr<effect> find_effect(const io::const_string& id) const noexcept;

	void add_surface(io::const_string&& id,surface&& surface);
	std::pair<bool,surface> find_surface(const io::const_string& sid) const noexcept;

	void add_sampler_ref(io::const_string&& id,io::const_string&& sid);
	io::const_string find_sampler_ref(const io::const_string& id) const noexcept;

private:
	effects_lib_t effects_;
	surfaces_lib_t surfaces_;
	sampler_refs_lib_t sampler_refs_;
};

DECLARE_IPTR(effect_library);


/// COLLADA assets model  e.g. meshes/patches and bound materials,texture images etc all in one
class model final:public io::object {
	model(const model&) = delete;
	model& operator=(const model&) = delete;
private:
	typedef detail::param< std::shared_ptr<effect> >::param_library effect_library_t;
	typedef detail::param< s_geometry >::param_library geometry_library_t;
	typedef detail::param< io::const_string >::param_library image_library_t;
    // links from material library to the effect library
	typedef detail::param< io::const_string >::param_library material_library_t;
public:
	model(model&&) noexcept = default;
	model& operator=(model&&) = default;

	model();
	virtual ~model() noexcept;

	s_effect_library effects() const noexcept {
		return effects_;
	}

	void add_material_effect_link(io::const_string&& id,io::const_string&& effect_id);
	std::shared_ptr<effect> find_material(const io::const_string& material_id) const noexcept;

	void add_geometry(io::const_string&& id,s_geometry&& e);
	s_geometry find_geometry(const io::const_string& id) noexcept;

	void add_image(io::const_string&& id,io::const_string&& img);
	io::const_string find_image(const io::const_string& id) noexcept;

	s_scene scene() const noexcept {
		return scene_;
	}

	void set_scene(s_scene&& scn) noexcept {
        scene_ = std::move(scn);
	}

private:
	s_effect_library effects_;
	image_library_t images_;
	geometry_library_t meshes_;
	material_library_t materials_;
	s_scene scene_;
};

DECLARE_IPTR(model);

} // namespace collada

#endif // __COLLADA_HPP_INCLUDED__
