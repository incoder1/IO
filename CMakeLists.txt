cmake_minimum_required(VERSION 3.4)

project(io)

include_directories(include)
include_directories(include/net)
# for precompiled headers
include_directories(src) 

set(CMAKE_BINARY_DIR target)

file(GLOB PROJECT_SOURCE_PATH ${PROJECT_SOURCE_DIR}/src)
file(GLOB COMMON_SRC ${PROJECT_SOURCE_PATH}/*.cpp ${PROJECT_SOURCE_PATH}/net/*.cpp)


set(EXTRA_LIBS)

if( WIN32 OR MSYS OR CYGWIN)
	message( STATUS "Building IO for Microsoft Windows")
	include_directories(include/win)
	file(GLOB PLATFORM_SRC ${PROJECT_SOURCE_PATH}/win/*.cpp)
	add_definitions(-D_WIN32_WINNT=0x0600)
	if(MSYS OR CYGWIN)
		set(THREADING threadwin)
	endif()
	
	list(APPEND EXTRA_LIBS iconv.dll)
	list(APPEND EXTRA_LIBS Ws2_32)
elseif( UNIX AND NOT WIN32 AND NOT MSYS AND NOT CYGWIN)
	message( STATUS "Building IO for UNIX/POSIX ")
	include_directories(include/posix)
	file(GLOB PLATFORM_SRC ${PROJECT_SOURCE_PATH}/posix/*.cpp)
	set(THREADING thread)
	list(APPEND EXTRA_LIBS pthread)
endif()

find_package(Boost 1.23.0)
if(Boost_FOUND)
	include_directories(${Boost_INCLUDE_DIRS})
	add_definitions(-DIO_HAS_BOOST)
endif()

#GnuTLS is not exist for MS VC++ 
if( NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC" )
	find_package(GnuTLS 3.0)
	if(GNUTLS_FOUND)
		include_directories(${GNUTLS_INCLUDE_DIR})
		list(APPEND EXTRA_LIBS ${GNUTLS_LIBRARIES})
		list(APPEND PLATFORM_SRC ${PROJECT_SOURCE_PATH}/tls/gnutls_secure_channel.cpp)
	endif()
endif()

set(DESTDIR lib)
add_library(io ${COMMON_SRC} ${PLATFORM_SRC})
target_link_libraries(io ${EXTRA_LIBS} )

if(BUILD_SHARED_LIBS)
  add_definitions(-DIO_SHARED_LIB)
  add_definitions(-DIO_BUILD)
endif()

if( ${CMAKE_BUILD_TYPE} STREQUAL "Release" )
 add_definitions(-DNDEBUG)
 set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/Release/lib)
elseif(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
	set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/Debug/lib)
endif()

if( ${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
	if(CMAKE_CXX_CREATE_SHARED_LIBRARY)
		set_target_properties(io PROPERTIES LINKER_LANGUAGE CXX)
	else()
		set_target_properties(io PROPERTIES LINKER_LANGUAGE AR)
	endif()
	
	set(MMX_AND_SSE_FLAGS "-mmmx -msse -msse2 -mssse3 -msse3 -msse4 -mavx -mavx2")
	set(RELEASE_OPTIMIZE_FLAGS "-mtune=generic -s -O3 -minline-stringops-dynamically -fdeclone-ctor-dtor -fdevirtualize-at-ltrans -mmovbe")
		
	if(NO_EXCEPTIONS)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")
	endif()
	if(NO_RTTI)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
	endif()
	
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
	if( WIN32 OR MSYS OR CYGWIN AND CMAKE_CXX_CREATE_SHARED_LIBRARY )
		set(LTO_FLAGS "-flto -Wl,-allow-multiple-definition")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} ${LTO_FLAGS}")
		set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS} -s ${LTO_FLAGS} ${RELEASE_OPTIMIZE_FLAGS} ${MMX_AND_SSE_FLAGS}")
	else()
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} ${RELEASE_OPTIMIZE_FLAGS} ${MMX_AND_SSE_FLAGS}" )
		set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS} -s")
	endif()	
	
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -g -Og")
	
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	find_package(GnuICONV REQUIRED)
	if (NOT BUILD_SHARED_LIBS)
		foreach(flag_var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
			if(${flag_var} MATCHES "/MD")
			string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
			endif(${flag_var} MATCHES "/MD")
		endforeach(flag_var)
	endif()
	
endif()

