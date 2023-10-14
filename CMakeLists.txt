cmake_minimum_required(VERSION 3.19 FATAL_ERROR)

project(Solution_IO CXX)

include(CMakeDependentOption)

if( ${CMAKE_BUILD_TYPE} STREQUAL "Release" )
	set(BUILD_TYPE_RELEASE ON)
	set(BUILD_TYPE_DEBUG OFF)
elseif(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
	set(BUILD_TYPE_DEBUG ON)
	set(BUILD_TYPE_RELEASE OFF)
endif()

if (NOT BUILD_LIBRARY_TYPE)
	SET_PROPERTY(CACHE BUILD_LIBRARY_TYPE PROPERTY STRINGS Shared Static)
	if(BUILD_TYPE_RELEASE)
		set (BUILD_LIBRARY_TYPE "Shared" CACHE STRING "${BUILD_LIBRARY_TYPE_DESCR}" FORCE)
	else()
		set (BUILD_LIBRARY_TYPE "Static" CACHE STRING "${BUILD_LIBRARY_TYPE_DESCR}" FORCE)
	endif()
endif()

option(BUILD_TESTING "Build the testing tree." OFF)
if(BUILD_TESTING)
	find_package(GTest)
endif()

cmake_dependent_option(MUST_BUILD_TESTING "Build the test project" ON "${BUILD_TESTING};${GTest_FOUND}" OFF)

add_subdirectory(libio)

if(MUST_BUILD_TESTING)
	add_subdirectory(test)
endif()

