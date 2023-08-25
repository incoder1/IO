cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

project(Solution_IO CXX)

# set type of OCCT libraries
if (NOT BUILD_LIBRARY_TYPE)
	set (BUILD_LIBRARY_TYPE "Shared" CACHE STRING "${BUILD_LIBRARY_TYPE_DESCR}" FORCE)
	SET_PROPERTY(CACHE BUILD_LIBRARY_TYPE PROPERTY STRINGS Shared Static)
endif()

if ("${BUILD_LIBRARY_TYPE}" STREQUAL "Shared")
	set (BUILD_SHARED_LIBS ON)
endif()

add_subdirectory(libio)

option(BUILD_TESTING "Build the testing tree." OFF)

if(BUILD_TESTING)
	add_subdirectory(test)
endif()

