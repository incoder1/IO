cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

project(io-gtest-suite CXX)

if(UNIX AND NOT WIN32 AND NOT MSYS AND NOT CYGWIN)
	set(CMAKE_THREAD_LIBS_INIT "-lpthread")
	set(CMAKE_HAVE_THREADS_LIBRARY 1)
	set(CMAKE_USE_PTHREADS_INIT 1)
	set(THREADS_PREFER_PTHREAD_FLAG ON)
endif()

find_package(Threads REQUIRED)
find_package(GTest REQUIRED)
include_directories(${GTest_INCLUDE_DIRS})

# Build test suite
include_directories(${PROJECT_SOURCE_DIR}/../libio/include)

file(GLOB TEST_SRC ${PROJECT_SOURCE_DIR}/*.cpp)
add_executable(io-gtest-suite ${TEST_SRC})
target_link_libraries(io-gtest-suite PRIVATE Threads::Threads ${GTEST_BOTH_LIBRARIES} io)

if(MSVC)
    add_definitions(/D "_WIN32_WINNT=0x0A00" /D "_UNICODE" /D "UNICODE" /D "_CRT_SECURE_NO_WARNINGS")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /c /nologo /utf-8 /std:c++latest /permissive- /Zc:wchar_t ")
    if(${CMAKE_BUILD_TYPE} STREQUAL "Release" )
        set_property(TARGET io-gtest-suite PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDLL$<$<CONFIG:Release>:Release>")
    else()
        set_property(TARGET io-gtest-suite PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL$<$<CONFIG:Debug>:Debug>")
    endif()
    set(Iconv_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/deps/msvc/include")
    set(GNUTLS_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/deps/msvc/include")
    link_directories(io "${CMAKE_SOURCE_DIR}/deps/msvc/lib/${CMAKE_VS_PLATFORM_TOOLSET_HOST_ARCHITECTURE}")
    set(Iconv_LIBRARY iconv)
    set(GNUTLS_LIBRARIES gnutls)
    if ("${BUILD_LIBRARY_TYPE}" STREQUAL "Shared")
       add_definitions(/D "IO_SHARED_LIB")
    endif()
else()
    if("${BUILD_LIBRARY_TYPE}" STREQUAL "Static")
        find_package(Iconv REQUIRED)
        find_package(GnuTLS 3.6.5 REQUIRED)
    else()
        add_definitions(/D "IO_SHARED_LIB")
    endif()
endif()

if( CMAKE_COMPILER_IS_GNUCXX )
	set_target_properties(io-gtest-suite PROPERTIES LINKER_LANGUAGE CXX)
endif()

#Run tests
enable_testing()

add_test(NAME io-check
		COMMAND io-gtest-suite
		WORKING_DIRECTORY ${LIBRARY_OUTPUT_PATH})

add_custom_command(
		TARGET io-gtest-suite
		COMMENT "Run tests"
		POST_BUILD
		COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --gtest_color=auto)
		
if(BUILD_SHARED_LIBS)
	if(WIN32)
		set_tests_properties(io-check PROPERTIES ENVIRONMENT_MODIFICATION PATH=path_list_append:${CMAKE_BINARY_DIR}/lib)
	elseif(UNIX)
		set_tests_properties(io-check PROPERTIES ENVIRONMENT_MODIFICATION LD_LIBRARY_PATH=path_list_append:${CMAKE_BINARY_DIR}/lib)
	endif()
endif()
