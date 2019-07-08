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
