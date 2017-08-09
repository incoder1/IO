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
  add_definitions(-DIO_SHARED_LIB -DIO_BUILD)
endif()


if(CMAKE_COMPILER_IS_GNUCXX)
	if(CMAKE_CXX_CREATE_SHARED_LIBRARY)
	  set_target_properties(libio PROPERTIES LINKER_LANGUAGE CXX)
	else()
		 set_target_properties(libio PROPERTIES LINKER_LANGUAGE AR)
	endif()
	include_directories(src) # for precompiled headers
	if(MINGW)
		include_directories(deps/include)
		link_directories(deps/lib)
		add_library(iconv.dll SHARED IMPORTED)
		# set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} -shared -Wl,--dll -Wl,--output-def=$(TARGET)/libio.def -Wl,--out-implib=$(CMAKE+)/libio.a")
	else()
		set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -pthread")
	endif()
	
	set(MMX_AND_SSE_FLAGS "-mmmx -msse -msse2 -msse3 -mssse3 -msse4" INTERNAL)
	set(RELEASE_OPTIMIZE_FLAGS "-mtune=generic -O3 -fno-exceptions -fno-rtti -minline-stringops-dynamically -fdeclone-ctor-dtor -fdevirtualize-at-ltrans -mmovbe" INTERNAL)
	
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++14" )
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -DNDEBUG ${RELEASE_OPTIMIZE_FLAGS} ${MMX_AND_SSE_FLAGS}" )
	set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS} -O3 -s")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -g -Og" )
endif()