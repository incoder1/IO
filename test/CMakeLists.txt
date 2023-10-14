cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

project(io-gtest-suite CXX)

include(CTest)

if(UNIX AND NOT WIN32 AND NOT MSYS AND NOT CYGWIN)
	set(CMAKE_THREAD_LIBS_INIT "-lpthread")
	set(CMAKE_HAVE_THREADS_LIBRARY 1)
	set(CMAKE_USE_PTHREADS_INIT 1)
	set(THREADS_PREFER_PTHREAD_FLAG ON)
else()
	if(CMAKE_COMPILER_IS_GNUCXX)
		set(THREADING threadwin)
	endif()
endif()


find_package(Threads REQUIRED)
find_package(GTest REQUIRED)
include_directories(${GTest_INCLUDE_DIRS})

# Build test suite
include_directories(${PROJECT_SOURCE_DIR}/../libio/include)

if ("${BUILD_LIBRARY_TYPE}" STREQUAL "Shared")
	set (BUILD_SHARED_LIBS ON)
	message("Linking tests with shared library")
else()
	message("Linking tests with static library")
endif()

file(GLOB TEST_SRC ${PROJECT_SOURCE_DIR}/*.cpp)
add_executable(io-gtest-suite ${TEST_SRC})

if(CMAKE_COMPILER_IS_GNUCXX)

	if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 11.0)
		set (CMAKE_CXX_STANDARD 23)
	elseif(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 9.0)
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

	set(LTO_FLAGS "-flto")

	if(WIN32 OR MSYS OR CYGWIN)
		add_definitions(-D_WIN32_WINNT=0x0A00 -D_UNICODE -DUNICODE)
		set(LTO_FLAGS "${LTO_FLAGS} -Wl,-allow-multiple-definition")
	endif()

	if(${CMAKE_BUILD_TYPE} STREQUAL "Release" )
		if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
			set(CPU_SPECIFIC_FLAGS "-minline-stringops-dynamically -mavx -mavx2 -mlzcnt -mmovbe")
			set(CPU_SPECIFIC_FLAGS "-minline-stringops-dynamically -sse2")
		elseif(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "i686")
		else()
			set(CPU_SPECIFIC_FLAGS "")
		endif()
		set(RELEASE_OPTIMIZE_FLAGS "-mtune=native -O3 -fdeclone-ctor-dtor -fdevirtualize-at-ltrans -fwhole-program ${CPU_SPECIFIC_FLAGS}")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} ${LTO_FLAGS}")
		set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} ${LTO_FLAGS} ${RELEASE_OPTIMIZE_FLAGS}")
	else()
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -g -Og -ggdb")
	endif()
	
	find_package(Iconv REQUIRED)
	find_package(GnuTLS 3.6.5 REQUIRED)
	include_directories(${Iconv_INCLUDE_DIR} ${GNUTLS_INCLUDE_DIR})
	set_target_properties(io-gtest-suite PROPERTIES LINKER_LANGUAGE CXX)
	link_directories(io "${CMAKE_BINARY_DIR}/lib")

	if(BUILD_SHARED_LIBS)
		add_definitions(-DIO_SHARED_LIB)
		if(WIN32 OR MSYS OR CYGWIN)
			message("Linking tests with DLL")
			target_link_libraries(io-gtest-suite PRIVATE Threads::Threads ${GTEST_BOTH_LIBRARIES} ${CMAKE_BINARY_DIR}/lib/libio.dll.a)
		else()
			target_link_libraries(io-gtest-suite PRIVATE Threads::Threads ${GTEST_BOTH_LIBRARIES} io)
		endif()
	else()
		target_link_libraries(io-gtest-suite PRIVATE Threads::Threads ${Iconv_LIBRARY} ${GNUTLS_LIBRARIES} ${GTEST_BOTH_LIBRARIES} io)
	endif()

endif(CMAKE_COMPILER_IS_GNUCXX)

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
	if (BUILD_SHARED_LIBS)
		add_definitions(/D "IO_SHARED_LIB")
		target_link_libraries(io-gtest-suite PRIVATE Threads::Threads ${GTEST_BOTH_LIBRARIES} io.dll)
	else()
		target_link_libraries(io-gtest-suite PRIVATE Threads::Threads ${Iconv_LIBRARY} ${GNUTLS_LIBRARIES} ${GTEST_BOTH_LIBRARIES} ${CMAKE_BINARY_DIR}/lib/libio.a)
	endif()
endif(MSVC)

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
