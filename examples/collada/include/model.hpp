#ifndef __MODEL_HPP_INCLUDED__
#define __MODEL_HPP_INCLUDED__

#include <object.hpp>

#include "vbo.hpp"
#include "shader.hpp"
#include "scene.hpp"

namespace engine {


class model: public io::object
{
protected:
	constexpr model() noexcept:
		io::object()
	{}
public:
	virtual void draw(const scene& scn) const = 0;
};

class mesh_model:public model {
public:
	mesh_model();
	virtual ~mesh_model() override;
	virtual void draw(const scene& scn) const override;
private:
    gl::s_program program_;
};

} // namespace engine

#endif // __MODEL_HPP_INCLUDED__
