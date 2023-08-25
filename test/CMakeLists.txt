cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

project(io-gtest-suite CXX)

include(CMakeDependentOption)
include(CTest)

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

if(MSVC)
	add_definitions(/D "_WIN32_WINNT=0x0A00" /D "_UNICODE" /D "UNICODE" /D "_CRT_SECURE_NO_WARNINGS")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /c /nologo /utf-8 /std:c++latest /permissive- /Zc:wchar_t")
	if(${CMAKE_BUILD_TYPE} STREQUAL "Release" )
		set_property(TARGET io-gtest-suite PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDLL$<$<CONFIG:Release>:Release>")
	else()
		set_property(TARGET io-gtest-suite PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL$<$<CONFIG:Debug>:Debug>")
	endif()
	set(Iconv_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/deps/msvc/include")
	set(Iconv_LIBRARY iconv)
	set(GNUTLS_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/deps/msvc/include")
	set(GNUTLS_LIBRARIES gnutls)
	link_directories(io "${CMAKE_SOURCE_DIR}/deps/msvc/lib/${CMAKE_VS_PLATFORM_TOOLSET_HOST_ARCHITECTURE}")
	if ("${BUILD_LIBRARY_TYPE}" STREQUAL "Shared")
		add_definitions(/D "IO_SHARED_LIB")
		target_link_libraries(io-gtest-suite PRIVATE Threads::Threads ${GTEST_BOTH_LIBRARIES} io)
	else()
		target_link_libraries(io-gtest-suite PRIVATE Threads::Threads ${Iconv_LIBRARY} ${GNUTLS_LIBRARIES} ${GTEST_BOTH_LIBRARIES} io)
	endif()
endif(MSVC)

if(CMAKE_COMPILER_IS_GNUCXX)
	if(WIN32 OR MSYS OR CYGWIN)
		add_definitions(-D_WIN32_WINNT=0x0A00 -D_UNICODE -DUNICODE)
	endif()
	if(${CMAKE_BUILD_TYPE} STREQUAL "Release" )
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -flto")
		set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -mtune=generic -O3 -fdeclone-ctor-dtor -fdevirtualize-at-ltrans -fwhole-program -flto")
	endif()
	find_package(Iconv REQUIRED)
	find_package(GnuTLS 3.6.5 REQUIRED)
	set_target_properties(io-gtest-suite PROPERTIES LINKER_LANGUAGE CXX)
	if("${BUILD_LIBRARY_TYPE}" STREQUAL "Shared")
		add_definitions(-DIO_SHARED_LIB)
		target_link_libraries(io-gtest-suite PRIVATE Threads::Threads ${GTEST_BOTH_LIBRARIES} io)
	else()
		target_link_libraries(io-gtest-suite PRIVATE Threads::Threads ${Iconv_LIBRARY} ${GTEST_BOTH_LIBRARIES}  ${GNUTLS_LIBRARIES} io)
	endif()
endif(CMAKE_COMPILER_IS_GNUCXX)

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
