#include "stdafx.hpp"
#include "nurb.hpp"
#include <glm/gtc/type_ptr.hpp>

namespace engine {

NURB::NURB(const io::s_read_channel vtx,const io::s_read_channel& frag,const io::s_read_channel& tess,const io::s_read_channel& tessctrl):
	surface()
{}

NURB::~NURB()
{}

} // namespace engine
