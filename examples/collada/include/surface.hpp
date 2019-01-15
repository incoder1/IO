#ifndef __SURFACE_HPP_INCLUDED__
#define __SURFACE_HPP_INCLUDED__

#include <object.hpp>
#include "scene.hpp"

namespace engine {

class surface: public io::object
{
	surface(const surface&) = delete;
	surface& operator=(const surface&) = delete;
protected:
	surface() noexcept;
public:
	virtual ~surface() noexcept = 0;
	virtual void draw(const scene& scn) const = 0;
};

DECLARE_IPTR(surface);

} // namespace engine

#endif // __SURFACE_HPP_INCLUDED__
