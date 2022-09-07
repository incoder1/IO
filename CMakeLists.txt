cmake_minimum_required(VERSION 3.4)

project(io CXX)

include(CMakeDependentOption)
include(CTest)

include_directories(include)
include_directories(include/net)
# for pre-compiled headers
include_directories(src)

file(GLOB PROJECT_SOURCE_PATH ${PROJECT_SOURCE_DIR}/src)
file(GLOB COMMON_SRC ${PROJECT_SOURCE_PATH}/*.cpp ${PROJECT_SOURCE_PATH}/net/*.cpp)
file(GLOB TLS_SRC "")

# set type of OCCT libraries
if (NOT BUILD_LIBRARY_TYPE)
	set (BUILD_LIBRARY_TYPE "Shared" CACHE STRING "${BUILD_LIBRARY_TYPE_DESCR}" FORCE)
	SET_PROPERTY(CACHE BUILD_LIBRARY_TYPE PROPERTY STRINGS Shared Static)
endif()

if ("${BUILD_LIBRARY_TYPE}" STREQUAL "Shared")
	set (BUILD_SHARED_LIBS ON)
endif()

set(EXTRA_LIBS)
set(CMAKE_BINARY_DIR target)

if( WIN32 OR MSYS OR CYGWIN)
	message( STATUS "Building IO for Microsoft Windows")
	include_directories(include/win)
	file(GLOB PLATFORM_SRC ${PROJECT_SOURCE_PATH}/win/*.cpp)
	if(MSVC)
		add_definitions(/D_WIN32_WINNT=0x0A00)
	else()
		add_definitions(-D_WIN32_WINNT=0x0A00)
	endif()
	if(MSYS OR CYGWIN)
		set(THREADING threadwin)
	endif()
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

# Dependencies
find_package(GnuTLS 3.6.5)
if(GNUTLS_FOUND)
	include_directories(${GNUTLS_INCLUDE_DIR})
	list(APPEND EXTRA_LIBS ${GNUTLS_LIBRARIES})
	include_directories(include/tls)
	file(GLOB TLS_SRC ${PROJECT_SOURCE_PATH}/tls/*.cpp)
endif(GNUTLS_FOUND) # GNU TLS


if(NOT MSVC)
	find_package(Iconv REQUIRED)
elseif(MSVC)
	set(Iconv_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/deps/msvc/include")
	if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
		set(Iconv_LIBRARY "${PROJECT_SOURCE_DIR}/deps/msvc/x64/iconv.lib")
	endif()
endif()
include_directories(${Iconv_INCLUDE_DIR})
list(APPEND EXTRA_LIBS ${Iconv_LIBRARY})

if( ${CMAKE_BUILD_TYPE} STREQUAL "Release" )
	if(MSVC)
		add_definitions(/DNDEBUG)
	else()
		add_definitions(-DNDEBUG)
	endif()
	set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/Release/lib)
elseif(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
	set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/Debug/lib)
endif()

if(BUILD_SHARED_LIBS)
	add_library(io SHARED ${COMMON_SRC} ${TLS_SRC} ${PLATFORM_SRC})
else()
	add_library(io STATIC ${COMMON_SRC} ${TLS_SRC} ${PLATFORM_SRC})
endif()

if( CMAKE_COMPILER_IS_GNUCXX )

	set(CMAKE_CXX_EXTENSIONS ON)

	target_link_libraries(io ${EXTRA_LIBS})
	target_precompile_headers(io PRIVATE ${PROJECT_SOURCE_PATH}/stdafx.hpp)
	set_target_properties(io PROPERTIES LINKER_LANGUAGE CXX)

	if(BUILD_SHARED_LIBS)
		add_definitions(-DIO_SHARED_LIB -DIO_BUILD)
	endif()

	if( ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
		set(CPU_SPECIFIC_FLAGS "-minline-stringops-dynamically -mavx -mavx2 -mlzcnt -mmovbe")
	else()
		set(CPU_SPECIFIC_FLAGS "")
	endif()

	set(RELEASE_OPTIMIZE_FLAGS "-mtune=generic -O3 -fdeclone-ctor-dtor -fdevirtualize-at-ltrans -fwhole-program ${CPU_SPECIFIC_FLAGS}")

	if(NO_EXCEPTIONS)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")
	endif()
	if(NO_RTTI)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
	endif()

	if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 9.0)
		set (CMAKE_CXX_STANDARD 20)
	elseif(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 4.9)
		set (CMAKE_CXX_STANDARD 17)
	elseif(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 4.8)
		set (CMAKE_CXX_STANDARD 14)
	elseif(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 4.1)
		set (CMAKE_CXX_STANDARD 11)
	else()
		message( SEND_ERROR "GCC 4.2+ required")
	endif()

	if( (WIN32 OR MSYS OR CYGWIN) AND BUILD_SHARED_LIBS )
		set(LTO_FLAGS "-flto -Wl,-allow-multiple-definition")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} ${LTO_FLAGS}")
		set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS} -s ${LTO_FLAGS} ${RELEASE_OPTIMIZE_FLAGS}")
		set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} ${LTO_FLAGS} ${RELEASE_OPTIMIZE_FLAGS}")
	else()
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} ${RELEASE_OPTIMIZE_FLAGS}" )
	endif()

	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -g -Og")

elseif(MSVC)
	target_precompile_headers(io PRIVATE ${PROJECT_SOURCE_PATH}/stdafx.hpp)

	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /c /nologo /utf-8 /std:c++latest")
	set(CMAKE_CXX_FLAGS_RELEASE  "${CMAKE_CXX_FLAGS} /O2 /Oi /Gw /Zc:wchar_t")

	add_definitions(/DUNICODE)

	if(BUILD_SHARED_LIBS)
		set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
		add_definitions(/DIO_SHARED_LIB)
		add_definitions(/DIO_BUILD)
		list(APPEND EXTRA_LIBS msvcrt.lib)
		list(APPEND EXTRA_LIB vcruntime.lib)
		target_link_libraries(io ${EXTRA_LIBS})
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} /Zl")
		set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /LTCG")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MD")
		target_link_libraries(io ${EXTRA_LIBS})
	endif()

	if(NOT NO_EXCEPTIONS OR BUILD_SHARED_LIBS)
		if(BUILD_SHARED_LIBS AND NO_EXCEPTIONS)
			message(SEND_ERROR "Could not build no exceptions DLL from Cmake, use nmake -f NMakefile-msvcx64-dll.mak instead")
		endif()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
	endif()

	if(NOT NO_RTTI)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GR")
	endif()

	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} /ZI")

endif()

if(RUN_TESTS)
	# Locate GTest
	find_package(GTest REQUIRED)
	include_directories(${GTest_INCLUDE_DIRS})
	if(IO_SHARED_LIB)
		if(MSVC)
			add_definitions(/DIO_SHARED_LIB)
			add_definitions(/DUNICODE)
		else()
			add_definitions(-DIO_SHARED_LIB)
			add_definitions(-DUNICODE)
		endif()
		if(WIN32)
			set(RUNTIME_PATH "$ENV{PATH};${LIBRARY_OUTPUT_PATH}")
			set_tests_properties(io_gtests PROPERTIES ENVIRONMENT  "PATH=${RUNTIME_PATH}")
		elseif(UNIX)
			set(RUNTIME_PATH "$ENV{LD_LIBRARY_PATH};${SO_DIR}")
			set_tests_properties(io_gtests PROPERTIES ENVIRONMENT "LD_LIBRARY_PATH=${RUNTIME_PATH}")
		endif()
	endif()
	if( ${CMAKE_BUILD_TYPE} STREQUAL "Release" )
		if(MSVC)
			add_definitions(/DNDEBUG)
		else()
			add_definitions(-DNDEBUG)
		endif()
	endif()
	link_directories(${LIBRARY_OUTPUT_PATH})
	file(GLOB TEST_SRC ${PROJECT_SOURCE_DIR}/test/*.cpp)
	add_executable(io-gtest-suite ${TEST_SRC})
	target_link_libraries(io-gtest-suite PRIVATE io  ${GTEST_BOTH_LIBRARIES})
	set_target_properties(io-gtest-suite PROPERTIES LINKER_LANGUAGE CXX)
	enable_testing()
  	add_test(NAME io-check
    	COMMAND io-gtest-suite
    	WORKING_DIRECTORY ${LIBRARY_OUTPUT_PATH})

	add_custom_command(
				   TARGET io-gtest-suite
				   COMMENT "Run tests"
                   POST_BUILD
                   COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure )
endif()
