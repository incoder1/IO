#include "view.hpp"

#include <iostream>

static const float COLORED_QUBE_VERTEX[216] = {
	// Coordinate  | Color | Normal

	// Top Quad
	1.0F, 1.0F, -1.0F,	0.5F, 0.5f, 0.5f,	0.0F, 1.0F, 0.0F,
	-1.0F, 1.0F, -1.0F,	0.5f, 0.5f, 0.5f,	0.0F, 1.0F, 0.0F,
	-1.0F, 1.0F, 1.0F,	0.5f, 0.5f, 0.5f,	0.0F, 1.0F, 0.0F,
	1.0F, 1.0F, 1.0F,	0.5f, 0.5f, 0.5f,	0.0F, 1.0F, 0.0F,

	// Bottom Quad
	1.0F, -1.0F, 1.0F,	0.0F, 1.0F, 0.0F,	 0.0F,-1.0F, 0.0F,
	-1.0F,-1.0F, 1.0F,	0.0F, 1.0F,0.0F,	 0.0F,-1.0F, 0.0F,
	-1.0F,-1.0F,-1.0F,	0.0F, 1.0F, 0.0F,	 0.0F,-1.0F, 0.0F,
	1.0F,-1.0F,-1.0F,	0.0F, 1.0F, 0.0F,	 0.0F,-1.0F, 0.0F,

	// Front Quad
	1.0F, 1.0F, 1.0F,	1.0F, 0.0F, 0.0F,	0.0F, 0.0F, 1.0F,
	-1.0F, 1.0F, 1.0F,	1.0F, 0.0F, 0.0F,	0.0F, 0.0F, 1.0F,
	-1.0F, -1.0F, 1.0F,	1.0F, 0.0F, 0.0F,	0.0F, 0.0F, 1.0F,
	1.0F, -1.0F, 1.0F,	1.0F, 0.0F, 0.0F,	0.0F, 0.0F, 1.0F,

	// Back Quad
	1.0F, -1.0F, -1.0F,	1.0F, 1.0F, 0.0F,	0.0F, 0.0F, -1.0F,
	-1.0F, -1.0F, -1.0F,	1.0F, 1.0F, 0.0F,	0.0F, 0.0F, -1.0F,
	-1.0F,  1.0F, -1.0F,	1.0F, 1.0F, 0.0F,	0.0F, 0.0F, -1.0F,
	1.0F,  1.0F, -1.0F,	1.0F, 1.0F, 0.0F,	0.0F, 0.0F, -1.0F,

	// Left Quad
	-1.0F, 1.0F, 1.0F,	0.0F, 0.0F, 1.0F,	-1.0F, 0.0F, 0.0F,
	-1.0F, 1.0F,-1.0F,	0.0F, 0.0F, 1.0F,	-1.0F, 0.0F, 0.0F,
	-1.0F,-1.0F,-1.0F,	0.0F, 0.0F,	1.0F,	-1.0F, 0.0F, 0.0F,
	-1.0F,-1.0F, 1.0F,	0.0F, 0.0F, 1.0F,	-1.0F, 0.0F, 0.0F,

	// Right Quad
	1.0F, 1.0F,-1.0F,	1.0F, 0.0F, 1.0F,	1.0F, 0.0F, 0.0F,
	1.0F, 1.0F, 1.0F,	1.0F, 0.0F, 1.0F,	1.0F, 0.0F, 0.0F,
	1.0F,-1.0F, 1.0F,	1.0F, 0.0F, 1.0F,	1.0F, 0.0F, 0.0F,
	1.0F,-1.0F,-1.0F,	1.0F, 0.0F, 1.0F,	1.0F, 0.0F, 0.0F
};

// TRANGLE_FAN imitation
static const uint32_t CUBE_INDEX[36] = {
	0,1,2,     0,2,3,
    4,5,6,     4,6,7,
    8,9,10,    8,10,11,
	12,13,14,  12,14,15,
    16,17,18,  16,18,19,
    20,21,22,  20,22,23
};

#ifdef _WIN32
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
#else
int main(int argc, const char** argv)
#endif // _WIN32
{
	if ( GLFW_TRUE == ::glfwInit() ) {
		try {
			engine::frame_view view(640,480,"Collada model view");
			engine::s_model md( new engine::untextured_static_mesh(COLORED_QUBE_VERTEX,216,CUBE_INDEX,36) );
			view.show(md);

			return 0;
		} catch(std::exception& exc) {
            std::cerr<< exc.what() << std::endl;
		}
		::glfwTerminate();
	}
	return -1;
}

