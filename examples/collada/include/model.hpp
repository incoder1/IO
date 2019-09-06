#ifndef __MODEL_HPP_INCLUDED__
#define __MODEL_HPP_INCLUDED__

#include <forward_list>

#include "surface.hpp"

namespace engine {

/// Rendering engine model e.g. combination of 3D graphical primitives (surfaces)
class model:public io::object {
public:
	/// Constructs new empty model
	model();
	virtual ~model() noexcept;
	/// Adds a surface to the model
	/// \param srf a surface to be added
	void add_surface(s_surface&& srf);
	/// Renders all surfaces together
	/// \param scn 3D scene to render the model
	void render(const scene& scn) const;
public:
	std::forward_list<s_surface> surfaces_;
};

DECLARE_IPTR(model);



} // namespace engine

#endif // __MODEL_HPP_INCLUDED__
