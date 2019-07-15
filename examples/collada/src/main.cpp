#include "stdafx.hpp"

#include "view.hpp"

#include "model.hpp"
#include "model_loader.hpp"


#ifndef NDEBUG
#	include <iostream>
#endif // NDEBUG


#ifdef __IO_WINDOWS_BACKEND__

#include <windows.h>

static io::file open_file_dialog()
{
	::OPENFILENAMEW ofn;
	wchar_t file_name[MAX_PATH] = { L'\0' };
	io_zerro_mem( &ofn, sizeof(ofn) );
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = nullptr;
    ofn.lpstrFile = file_name;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = L"COLLADA\0*.DAE\0";
    ofn.nFilterIndex = 0;
    ofn.lpstrFileTitle = nullptr;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = L"models";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    return TRUE == ::GetOpenFileNameW(&ofn) ?  io::file( ofn.lpstrFile ) : io::file("");
}

#endif // __IO_WINDOWS_BACKEND__


// parses COLLADA file and loads it into OpenGL rendering engine
static void load_collada_model(engine::s_model& dst_mdl,io::file&& dae)
{
#ifndef NDEBUG
	std::cout << "About to load " << dae.path() << " COLLADA file" << std::endl;
#endif // NDEBUG
	engine::model_loader ldr(dae);
	ldr.load(dst_mdl);
}

#ifdef _WIN32
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
#else
int main(int argc, const char** argv)
#endif // _WIN32
{

	if ( GLFW_TRUE == ::glfwInit() ) {
		try {
			engine::frame_view view(640,480,"Collada model view");

			//engine::s_model mdl( new engine::model() );
			//mdl->add_surface( std::move(cube) );

			engine::s_model mdl( new engine::model() );
			//load_collada_model(mdl, "models/textured_cube/text_cube.dae");
			//load_collada_model(mdl, "models/RubiksCube/rubik_cube.dae");
			//load_collada_model(mdl,"models/Earth/Earth.dae");
#ifdef __IO_WINDOWS_BACKEND__
			load_collada_model(mdl, open_file_dialog() );
#else
			load_collada_model(mdl, io::file(argv[1]) );
#endif
			view.show( mdl );
			return 0;
		}
		catch(std::exception& exc) {
#ifndef NDEBUG
			std::cerr<< exc.what();
			std::cerr.flush();
#endif // NDEBUG
		}
		::glfwTerminate();
	}
	return -1;
}

