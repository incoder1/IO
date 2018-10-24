#include "view.hpp"

#include <iostream>

#ifdef _WIN32
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
#else
int main(int argc, const char** argv)
#endif // _WIN32
{
	if ( GLFW_TRUE == ::glfwInit() ) {
		try {
			engine::frame_view view(640,480,"Collada model view");
			view.show(nullptr);
			return 0;
		} catch(std::exception& exc) {
            std::cerr<< exc.what() << std::endl;
		}
		::glfwTerminate();
	}
	return -1;
}

