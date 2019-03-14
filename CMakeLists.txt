cmake_minimum_required(VERSION 3.4)

project(io CXX)

include_directories(include)
include_directories(include/net)
# for pre-compiled headers
include_directories(src)


file(GLOB PROJECT_SOURCE_PATH ${PROJECT_SOURCE_DIR}/src)
file(GLOB COMMON_SRC ${PROJECT_SOURCE_PATH}/stdafx.hpp ${PROJECT_SOURCE_PATH}/*.cpp ${PROJECT_SOURCE_PATH}/net/*.cpp)

set(EXTRA_LIBS)
set(CMAKE_BINARY_DIR target)

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
	if(MSVC)
		add_definitions(/DIO_HAS_BOOST)
	else()
		add_definitions(-DIO_HAS_BOOST)
	endif()
endif()

#GnuTLS is not exist for MS VC++
if( NOT MSVC )
	find_package(GnuTLS 3.0)
	if(GNUTLS_FOUND)
	   add_definitions(-DIO_TLS_PROVIDER_GNUTSL ${GNUTLS_DEFINITIONS} )
	   include_directories(${GNUTLS_INCLUDE_DIR})
	   list(APPEND EXTRA_LIBS ${GNUTLS_LIBRARIES})
	   list(APPEND PLATFORM_SRC ${PROJECT_SOURCE_PATH}/tls/gnutls_secure_channel.cpp)
	endif() # GNU TLS
else()
	list(APPEND EXTRA_LIBS msvcrt.lib)
	list(APPEND EXTRA_LIBS kernel32.lib)
	list(APPEND EXTRA_LIBS user32.lib)
	list(APPEND EXTRA_LIBS Ws2_32.lib)
	list(APPEND EXTRA_LIBS iconv.dll.lib)
endif()



if( ${CMAKE_BUILD_TYPE} STREQUAL "Release" )
 add_definitions(-DNDEBUG)
 set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/Release/lib)
elseif(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
	set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/Debug/lib)
endif()

if(BUILD_SHARED_LIBS)
	add_library(io SHARED ${COMMON_SRC} ${PLATFORM_SRC})
else()
	add_library(io STATIC ${COMMON_SRC} ${PLATFORM_SRC})
endif()

target_link_libraries(io ${EXTRA_LIBS})

if( CMAKE_COMPILER_IS_GNUCXX )

	if(BUILD_SHARED_LIBS)
		add_definitions(-DIO_SHARED_LIB -DIO_BUILD)
		set_target_properties(io PROPERTIES LINKER_LANGUAGE CXX)
	endif()

	if( ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
		set(CPU_SPECIFIC_FLAGS "-minline-stringops-dynamically -mavx -mavx2 -mlzcnt -mmovbe")
	else()
		set(CPU_SPECIFIC_FLAGS "")
	endif()

	set(RELEASE_OPTIMIZE_FLAGS "-mtune=generic -s -O3 -fdeclone-ctor-dtor -fdevirtualize-at-ltrans -fwhole-program ${CPU_SPECIFIC_FLAGS}")

	if(NO_EXCEPTIONS)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")
	endif()
	if(NO_RTTI)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
	endif()

	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
	if( (WIN32 OR MSYS OR CYGWIN) AND BUILD_SHARED_LIBS )
		set(LTO_FLAGS "-flto -Wl,-allow-multiple-definition")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} ${LTO_FLAGS}")
		set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS} -s ${LTO_FLAGS} ${RELEASE_OPTIMIZE_FLAGS}")
	else()
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} ${RELEASE_OPTIMIZE_FLAGS}" )
		set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS}")
	endif()

	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -g -Og")

elseif(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest /utf-8 /Zc:wchar_t")
	add_definitions(/DUNICODE)
	set(CMAKE_CXX_FLAGS_RELEASE  "${CMAKE_CXX_FLAGS} /Ox /Oi")
	if(BUILD_SHARED_LIBS)
		add_definitions(/DIO_SHARED_LIB /DIO_BUILD )
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} /GL")
		set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS} /DLL /LTCG")
	endif()
	if(NOT NO_EXCEPTIONS)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
	endif()
	if(NOT NO_RTTI)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GR")
	endif()
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} /ZI")
endif()


