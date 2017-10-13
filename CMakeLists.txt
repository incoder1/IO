cmake_minimum_required(VERSION 3.5)

project(io)

include_directories(include)
include_directories(include/net)

set(CMAKE_BINARY_DIR target)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)

file(GLOB PROJECT_SOURCE_PATH ${PROJECT_SOURCE_DIR}/src)
file(GLOB COMMON_SRC ${PROJECT_SOURCE_PATH}/*.cpp ${PROJECT_SOURCE_PATH}/net/*.cpp)

if( WIN32 OR MSYS OR CYGWIN)
	message( STATUS "Building IO for Microsoft Windows")
	include_directories(include/win)
	file(GLOB PLATFORM_SRC ${PROJECT_SOURCE_PATH}/win/*.cpp)
	add_definitions(-D_WIN32_WINNT=0x0600)
endif()

if( UNIX AND NOT WIN32 AND NOT MSYS AND NOT CYGWIN)
	message( STATUS "Building IO for UNIX/POSIX ")
	include_directories(include/posix)
	file(GLOB PLATFORM_SRC ${PROJECT_SOURCE_PATH}/posix/*.cpp)
endif()

find_package(Boost 1.23.0)
if(Boost_FOUND)
	include_directories(${Boost_INCLUDE_DIRS})
	add_definitions(-DIO_HAS_BOOST)
endif()

set(DESTDIR lib)

add_library(io ${COMMON_SRC} ${PLATFORM_SRC} )

if(BUILD_SHARED_LIBS)
  add_definitions(-DIO_SHARED_LIB)
  add_definitions(-DIO_BUILD)
endif()

if( "${CMAKE_BUILD_TYPE}" STREQUAL "Release" )
 add_definitions(-DNDEBUG)
endif()


if(CMAKE_COMPILER_IS_GNUCXX)
	if(CMAKE_CXX_CREATE_SHARED_LIBRARY)
		set_target_properties(io PROPERTIES LINKER_LANGUAGE CXX)
	else()
		set_target_properties(io PROPERTIES LINKER_LANGUAGE AR)
	endif()
	include_directories(src) # for precompiled headers
	if( WIN32 OR MSYS )
		message( STATUS "Using MSYS2 MINGW64 iconv.dll gnutls.dll Ws2_32")
		set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -liconv.dll -lgnutls.dll -lWs2_32")
	else()
		find_library(gnutls REQUIRED)
		set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}")
	endif()
	
	set(MMX_AND_SSE_FLAGS "-mmmx -msse -msse2 -mssse3 -msse3 -msse4 -mavx")
	set(RELEASE_OPTIMIZE_FLAGS "-mtune=generic -s -O3 -minline-stringops-dynamically -fdeclone-ctor-dtor -fdevirtualize-at-ltrans -mmovbe")
	
	if(NO_EXCEPTIONS)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")
	endif()
	if(NO_RTTI)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
	endif()
	
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++14 " )
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} ${RELEASE_OPTIMIZE_FLAGS} ${MMX_AND_SSE_FLAGS}" )
	set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS} -O3 -s")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -g -Og" )
endif()

