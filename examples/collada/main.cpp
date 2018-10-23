#include "view.hpp"

#ifdef _WIN32
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
#else
int main(int argc, const char** argv)
#endif // _WIN32
{
	if ( GLFW_TRUE == ::glfwInit() ) {
		::glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		::glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

		engine::frame_view view(640,480,"Collada model view");
		view.show(nullptr);

		::glfwTerminate();
		return 0;
	}
	return -1;
}

