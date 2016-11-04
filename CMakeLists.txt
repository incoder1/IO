cmake_minimum_required(VERSION 3.5)

project(libio)

include_directories(include)

file(GLOB_RECURSE libio_SOURCES "src/*.cpp")

if(WIN32)
	file(GLOB_RECURSE libio_platform_SOURCES "src/win/*.cpp")
else()
	file(GLOB_RECURSE libio_platform_SOURCES "src/posix/*.cpp")
endif()

find_package(Boost)
if(Boost_FOUND)
	include_directories(${Boost_INCLUDE_DIRS})
	add_definitions(-DIO_HAS_BOOST)
endif()



set(CMAKE_BUILD_TYPE release)
set(CMAKE_BUILD_TYPE debug)

add_library(libio ${libio_SOURCES} ${libio_platform_SOURCES} )

set(DESTDIR lib)

if(CMAKE_CXX_CREATE_SHARED_LIBRARY)
  add_definitions(-DIO_SHARED_LIB -DIO_BUILD_SHARED_LIB)
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
	if(CMAKE_CXX_CREATE_SHARED_LIBRARY)
	  set_target_properties(libio PROPERTIES LINKER_LANGUAGE CXX)
	else()
		 set_target_properties(libio PROPERTIES LINKER_LANGUAGE AR)
	endif()
	if(MINGW)
		include_directories(deps/include)
		link_directories(deps/lib)
		add_library(iconv.dll SHARED IMPORTED)
	endif()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++14 -Winline -pedantic")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -mtune=generic -Ofast -fno-exceptions -fno-rtti -DNDEBUG")
	set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS} -s")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -g -Og")
endif()