/*
 *
 * Copyright (c) 2016-2019
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
/**
 * Parses a COLLADA XML file model and display it using a OpenGL based rendering engine
 * This is not full featured COLLADA model viewer, the propose is demonstrate using IO library
 * in useful and complex applications
 */
#include "stdafx.hpp"

#include "view.hpp"
#include "model.hpp"
#include "model_loader.hpp"

#include "nurb.hpp"
#include "polymesh.hpp"

#ifndef NDEBUG
#	include <iostream>
#	include <chrono>
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
	auto start = std::chrono::steady_clock::now();
#endif // NDEBUG
	engine::model_loader ldr(dae);
	ldr.load(dst_mdl);
#ifndef NDEBUG
	auto diff = std::chrono::steady_clock::now() - start;
	std::cout << "Parsing and loading time " << std::chrono::duration <double, std::milli>(diff).count() << " ms" << std::endl;
#endif // NDEBUG

}

// polygonal cube for test
static float CUBE[] = {
	// position | normal
	// left
	 1.0F, 1.0F, 1.0F, 1.0F, 0.0F, 0.0F,
	 1.0F, 1.0F,-1.0F, 1.0F, 0.0F, 0.0F,
	 1.0F,-1.0F,-1.0F, 1.0F, 0.0F, 0.0F,
	 1.0F,-1.0F, 1.0F, 1.0F, 0.0F, 0.0F,
	// front
	-1.0F, 1.0F, 1.0F, 0.0F, 0.0F, 1.0F,
	 1.0F, 1.0F, 1.0F, 0.0F, 0.0F, 1.0F,
	 1.0F,-1.0F, 1.0F, 0.0F, 0.0F, 1.0F,
	-1.0F,-1.0F, 1.0F, 0.0F, 0.0F, 1.0F,
	 // top
	-1.0F, 1.0F, 1.0F, 0.0F, 1.0F, 0.0F,
	-1.0F, 1.0F,-1.0F, 0.0F, 1.0F, 0.0F,
	 1.0F, 1.0F,-1.0F, 0.0F, 1.0F, 0.0F,
	 1.0F, 1.0F, 1.0F, 0.0F, 1.0F, 0.0F,
     // bottom
	-1.0F,-1.0F, 1.0F, 0.0F,-1.0F, 0.0F,
	-1.0F,-1.0F,-1.0F, 0.0F,-1.0F, 0.0F,
	 1.0F,-1.0F,-1.0F, 0.0F,-1.0F, 0.0F,
	 1.0F,-1.0F, 1.0F, 0.0F,-1.0F, 0.0F,
	// right
	-1.0F, 1.0F, 1.0F, -1.0F, 0.0F, 0.0F,
	-1.0F, 1.0F,-1.0F, -1.0F, 0.0F, 0.0F,
	-1.0F,-1.0F,-1.0F, -1.0F, 0.0F, 0.0F,
	-1.0F,-1.0F, 1.0F, -1.0F, 0.0F, 0.0F,
	// back
	-1.0F, 1.0F,-1.0F, 0.0F, 0.0F, -1.0F,
	 1.0F, 1.0F,-1.0F, 0.0F, 0.0F, -1.0F,
	 1.0F,-1.0F,-1.0F, 0.0F, 0.0F, -1.0F,
	-1.0F,-1.0F,-1.0F, 0.0F, 0.0F, -1.0F
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

			engine::s_model mdl( new engine::model() );

			//engine::s_surface nurb = engine::NURB::create(engine::DEFAULT_MATERIAL,quad, 48, 16, 4);
			//mdl->add_surface( std::move(nurb) );

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
			std::cerr<< exc.what() << std::endl;
#endif // NDEBUG
		}
		::glfwTerminate();
	}
	return -1;
}

