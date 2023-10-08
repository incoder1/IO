cmake_minimum_required(VERSION 3.19 FATAL_ERROR)

project(Solution_IO CXX)

# set type of OCCT libraries
if (NOT BUILD_LIBRARY_TYPE)
	set (BUILD_LIBRARY_TYPE "Shared" CACHE STRING "${BUILD_LIBRARY_TYPE_DESCR}" FORCE)
	SET_PROPERTY(CACHE BUILD_LIBRARY_TYPE PROPERTY STRINGS Shared Static)
endif()

add_subdirectory(libio)

option(BUILD_TESTING "Build the testing tree." OFF)

if(BUILD_TESTING)
	add_subdirectory(test)
endif()

