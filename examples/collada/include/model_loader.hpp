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
		model_loader(const io::file& src);
		~model_loader() noexcept;
		void load(s_model& dst);
	private:
		static collada::s_model parse_collada_xml(const io::file& src_file);
		s_surface load_static_sub_mesh(const collada::s_sub_mesh& sm, const collada::mesh* src_mesh, material_t&& mat);
		s_surface load_textured_sub_mesh(const collada::s_sub_mesh& sm, const collada::mesh* mesh,const io::const_string& texture);
		gl::s_texture get_texture_by_name(const io::const_string& name);
		void load_mesh(s_model& dst_mdl,const collada::mesh* src_mesh);
	private:
		io::file src_;
		collada::s_model src_mdl_;
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
