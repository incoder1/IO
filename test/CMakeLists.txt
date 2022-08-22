cmake_minimum_required(VERSION 3.4)

project(io_test CXX)

get_filename_component(LIB_BINARY_DIR ${PROJECT_SOURCE_DIR}/../target ABSOLUTE)

if( ${CMAKE_BUILD_TYPE} STREQUAL "Release" )	
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY target/Release)		
	set(CMAKE_INSTALL_RPATH ${LIB_BINARY_DIR}/Release/lib)
elseif(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY target/Debug)
	set(CMAKE_INSTALL_RPATH ${LIB_BINARY_DIR}/Debug/lib)
endif()


file(GLOB TEST_SRC ${PROJECT_SOURCE_DIR}/*.cpp)

# Locate GTest
find_package(GTest REQUIRED)

add_executable(io_test ${TEST_SRC} )

include_directories(${GTest_INCLUDE_DIRS})
target_link_libraries(io_test PRIVATE io ${GTEST_BOTH_LIBRARIES} )

target_precompile_headers(io_test PRIVATE ${PROJECT_SOURCE_PATH}/stdafx.hpp)

add_test(NAME io_gtests COMMAND io_test)

if(WIN32 OR MSYS OR CYGWIN)	
	cmake_path(CONVERT ${CMAKE_INSTALL_RPATH} TO_NATIVE_PATH_LIST DLL_DIR)
	set(RUNTIME_PATH "$ENV{PATH};${DLL_DIR}")
	set_tests_properties(io_gtests PROPERTIES ENVIRONMENT  "PATH=${RUNTIME_PATH}")
elseif(UNIX AND NOT WIN32 AND NOT MSYS AND NOT CYGWIN)
	cmake_path(CONVERT ${CMAKE_INSTALL_RPATH} TO_NATIVE_PATH_LIST SO_DIR)
	set(RUNTIME_PATH "$ENV{LD_LIBRARY_PATH};${SO_DIR}")
	set_tests_properties(io_gtests PROPERTIES ENVIRONMENT "LD_LIBRARY_PATH=${RUNTIME_PATH}")
endif()




