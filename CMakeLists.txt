cmake_minimum_required(VERSION 3.5)

project(io)

include_directories(include)
include_directories(include/net)

file(GLOB PROJECT_SOURCE_DIR src)
file(GLOB COMMON_SRC ${PROJECT_SOURCE_DIR}/*.cpp)
file(GLOB COMMON_SRC ${PROJECT_SOURCE_DIR}/net/*.cpp)

if( WIN32 OR MSYS OR CYGWIN)
	message( STATUS "Building IO for Microsoft Windows")
	include_directories(include/win)
	file(GLOB PLATFORM_SRC ${PROJECT_SOURCE_DIR}/win/*.cpp)
endif()

if( UNIX AND NOT WIN32 AND NOT MSYS AND NOT CYGWIN)
	message( STATUS "Building IO for UNIX/POSIX ")
	include_directories(include/posix)
	file(GLOB PLATFORM_SRC ${PROJECT_SOURCE_DIR}/posix/*.cpp)
endif()

find_package(Boost 1.23.0)
if(Boost_FOUND)
	include_directories(${Boost_INCLUDE_DIRS})
	add_definitions(-DIO_HAS_BOOST)
endif()

set(DESTDIR lib)

add_library(io ${COMMON_SRC} ${PLATFORM_SRC} )

if(CMAKE_CXX_CREATE_SHARED_LIBRARY)
  add_definitions(-DIO_SHARED_LIB -DIO_BUILD)
endif()


if(CMAKE_COMPILER_IS_GNUCXX)
	if(CMAKE_CXX_CREATE_SHARED_LIBRARY)
		set_target_properties(io PROPERTIES LINKER_LANGUAGE CXX)
	else()
		set_target_properties(io PROPERTIES LINKER_LANGUAGE AR)
	endif()
	include_directories(src) # for precompiled headers
	if(MSYS OR MINGW)
		include_directories(deps/include)
		link_directories(deps/lib)
		add_library(iconv.dll SHARED IMPORTED)
		add_library(tls-15.dll SHARED IMPORTED)
		add_library(ssl-43.dll SHARED IMPORTED)
		add_library(crypto-41.dll SHARED IMPORTED)
		add_library(Ws2_32 SHARED IMPORTED)
	else()
		set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -pthread")
		find_package(LibreSSL C REQUIRED)
	endif()
	
	set(MMX_AND_SSE_FLAGS "-mmmx -msse -msse2 -mssse3 -msse3 -msse4 -mavx" INTERNAL)
	set(RELEASE_OPTIMIZE_FLAGS "-mtune=generic -s -O3 -fno-exceptions -fno-rtti -minline-stringops-dynamically -fdeclone-ctor-dtor -fdevirtualize-at-ltrans -mmovbe" INTERNAL)
	
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++14 " )
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -DNDEBUG ${RELEASE_OPTIMIZE_FLAGS} ${MMX_AND_SSE_FLAGS}" )
	set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS} -O3 -s")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -g -Og" )
endif()

