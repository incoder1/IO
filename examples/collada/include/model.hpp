#ifndef __MODEL_HPP_INCLUDED__
#define __MODEL_HPP_INCLUDED__

#include <forward_list>

#include "surface.hpp"


namespace engine {

class model:public io::object {
public:
	model();
	virtual ~model() noexcept;
	void add_surface(s_surface&& srf);
	void render(const scene& scn) const;
public:
	std::forward_list<s_surface> surfaces_;
};

DECLARE_IPTR(model);



} // namespace engine

#endif // __MODEL_HPP_INCLUDED__
