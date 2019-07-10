#ifndef __MODEL_LOADER_HPP_INCLUDED__
#define __MODEL_LOADER_HPP_INCLUDED__

#include <string>
#include <unordered_map>

#include <files.hpp>

#include "image.hpp"
#include "model.hpp"
#include "mesh.hpp"
#include "parser.hpp"

#ifndef NDEBUG
#	include <iostream>
#endif // NDEBUG

namespace engine {

namespace detail {

class vertex_accessor {
public:
	constexpr vertex_accessor() noexcept:
		arr_(),
		offset_(0),
		stride_(0)
	{}
	std::size_t stride() const noexcept {
		return stride_;
	}
	std::size_t offset() const noexcept {
		return offset_;
	}
	void copy(float * dst, std::size_t index) const noexcept {
		const float* src = arr_.get() + (index * stride_);
        std::memmove(dst, src,  (stride_ * sizeof(float)) );
	}
private:
	friend class vertex_accessor_builder;
	collada::float_array arr_;
	std::size_t offset_;
	std::size_t stride_;
};

class vertex_accessor_builder {
public:
	vertex_accessor_builder() noexcept:
		ret_()
	{}
    void set_array(const collada::float_array& arr) noexcept {
		ret_.arr_ = arr;
    }
    void set_stride(std::size_t stride) noexcept {
    	ret_.stride_ = stride;
    }
    void set_offset(std::size_t offset) noexcept {
    	ret_.offset_ = offset;
    }
    vertex_accessor build() noexcept {
    	return ret_;
    }
private:
	vertex_accessor ret_;
};

class static_mesh_accessor
{
public:
	static_mesh_accessor(vertex_accessor&& pos, vertex_accessor&& nrm, collada::unsigned_int_array&& idx);
	void copy_vertex(float* dst, std::size_t vertex_index);
	std::size_t vertex_count() const noexcept {
		return vertex_count_;
	}
	std::size_t vertex_size() const noexcept {
		return vertex_size_;
	}
private:
	vertex_accessor position_;
	vertex_accessor normale_;
	collada::unsigned_int_array idx_;
	std::size_t vertex_count_;
	std::size_t vertex_size_;
};

struct textured_mesh_accessor
{
	textured_mesh_accessor(vertex_accessor&& pos, vertex_accessor&& nrm, vertex_accessor&& uv, collada::unsigned_int_array&& idx);
	void copy_vertex(float* dst, std::size_t vertex_index);
	std::size_t vertex_count() const noexcept {
		return vertex_count_;
	}
	std::size_t vertex_size() const noexcept {
		return vertex_size_;
	}
private:
	vertex_accessor position_;
	vertex_accessor normale_;
	vertex_accessor uv_;
	collada::unsigned_int_array idx_;
	std::size_t vertex_count_;
	std::size_t vertex_size_;
};

} // namespace detail

class model_loader
{
	public:
		/// Parses COLLADA XML file
		model_loader(const io::file& src);
		~model_loader() noexcept;
		/// Loads COLLADA model to the rendering engine
		/// \param dst rendering model, to fill from COLLADA
		void load(s_model& dst);
	private:
		static collada::s_model parse_collada_xml(const io::file& src_file);

		detail::static_mesh_accessor create_static_mesh_accessor(const collada::s_sub_mesh& sm, const collada::mesh* src_mesh);
		detail::textured_mesh_accessor create_textured_mesh_accessor(const collada::s_sub_mesh& sm, const collada::mesh* src_mesh);

		/// Loads geometry-material based mesh into engine's mesh renderer
		s_surface load_static_sub_mesh(const collada::s_sub_mesh& sm, const collada::mesh* src_mesh, material_t&& mat);
		/// Loads geometry-diffuse based mesh into engine's mesh renderer
		s_surface load_textured_sub_mesh(const collada::s_sub_mesh& sm, const collada::mesh* mesh,const io::const_string& texture);
		/// Loads PNG texture file into OpenGL video memory
		gl::s_texture get_texture_by_name(const io::const_string& name);
		/// Select mesh type and load it into rendering engine
		void load_mesh(s_model& dst_mdl,const collada::mesh* src_mesh);
	private:
		// basic COLLADA file
		io::file src_;
		// COLLADA parsed result
		collada::s_model src_mdl_;
		// texture's library <name,texture>, used across geometry primitives
		std::unordered_map<
			io::const_string,
			gl::s_texture,
			io::const_string_hash,
			std::equal_to<io::const_string>,
			std::allocator< std::pair<io::const_string,gl::s_texture> >
			> textures_;
};

} // namespace engine

#endif // MODEL_LOADER_H
