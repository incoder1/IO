cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

project(io CXX)

include(CMakeDependentOption)
include_directories(include)

file(GLOB CORE_MODULE_SRC_ROOT ${PROJECT_SOURCE_DIR}/src/core)
file(GLOB CORE_SRC ${CORE_MODULE_SRC_ROOT}/*.cpp)
file(GLOB CORE_SRC_POSIX ${CORE_MODULE_SRC_ROOT}/posix/*.cpp)
file(GLOB CORE_SRC_WINDOWS ${CORE_MODULE_SRC_ROOT}/win/*.cpp)

file(GLOB NET_MOUDLE_SRC_ROOT ${PROJECT_SOURCE_DIR}/src/net)
file(GLOB NET_SRC ${NET_MOUDLE_SRC_ROOT}/*.cpp)
file(GLOB NET_SRC_POSIX ${NET_MOUDLE_SRC_ROOT}/posix/*.cpp)
file(GLOB NET_SRC_WINDOWS ${NET_MOUDLE_SRC_ROOT}/win/*.cpp)

file(GLOB TEXTAPI_SRC ${PROJECT_SOURCE_DIR}/src/textapi/*.cpp)
file(GLOB TLS_SRC ${PROJECT_SOURCE_DIR}/src/tls/*.cpp)
file(GLOB XML_SRC ${PROJECT_SOURCE_DIR}/src/xml/*.cpp)

if ("${BUILD_LIBRARY_TYPE}" STREQUAL "Shared")
	set (BUILD_SHARED_LIBS ON)
endif()

set(EXTRA_LIBS)

if( WIN32 OR MSYS OR CYGWIN)
	set(THREADING threadwin)
	list(APPEND EXTRA_LIBS Ws2_32)
elseif( UNIX AND NOT WIN32 AND NOT MSYS AND NOT CYGWIN)
	message( STATUS "Building IO for UNIX/POSIX ")
	set(CMAKE_THREAD_LIBS_INIT "-lpthread")
	set(CMAKE_HAVE_THREADS_LIBRARY 1)
	set(CMAKE_USE_PTHREADS_INIT 1)
	set(THREADS_PREFER_PTHREAD_FLAG ON)
endif()

find_package(Threads REQUIRED)

find_package(Boost 1.23.0)
if(Boost_FOUND)
	include_directories(${Boost_INCLUDE_DIRS})
	if(MSVC)
		add_definitions(/DIO_HAS_BOOST)
	else()
		add_definitions(-DIO_HAS_BOOST)
	endif()
endif()

if( ${CMAKE_BUILD_TYPE} STREQUAL "Release" )
	set(BUILD_TYPE_RELEASE ON)
elseif(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
	set(BUILD_TYPE_DEBUG ON)
endif()

set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/${CMAKE_VS_PLATFORM_TOOLSET_HOST_ARCHITECTURE}/lib)

if( CMAKE_COMPILER_IS_GNUCXX )

	set(CMAKE_CXX_EXTENSIONS ON)
	
	if( ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
		set(CPU_SPECIFIC_FLAGS "-minline-stringops-dynamically -mavx -mavx2 -mlzcnt -mmovbe")
	elseif(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "i686")
		set(CPU_SPECIFIC_FLAGS "-minline-stringops-dynamically -sse2")
	else()
		set(CPU_SPECIFIC_FLAGS "")
	endif()

	set(RELEASE_OPTIMIZE_FLAGS "-mtune=generic -O3 -fdeclone-ctor-dtor -fdevirtualize-at-ltrans -fwhole-program ${CPU_SPECIFIC_FLAGS}")

	if(NO_EXCEPTIONS)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions")
	endif()

	if(NO_RTTI)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
	endif()

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

	if(WIN32 OR MSYS OR CYGWIN)
		add_definitions(-D_WIN32_WINNT=0x0A00 -D_UNICODE -DUNICODE)
	endif()
	
	# for pre-compiled headers
	include_directories(${PROJECT_SOURCE_DIR}/src)
	
	find_package(Iconv REQUIRED)
	include_directories(${Iconv_INCLUDE_DIR})
	list(APPEND EXTRA_LIBS ${Iconv_LIBRARY})

	find_package(GnuTLS 3.6.5 REQUIRED)
	include_directories(${GNUTLS_INCLUDE_DIR})
	list(APPEND EXTRA_LIBS ${GNUTLS_LIBRARIES})

	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -g -Og -ggdb")

	if( BUILD_TYPE_RELEASE )
		add_definitions(-DNDEBUG)
	endif()

	if(BUILD_SHARED_LIBS)
		add_definitions(-DIO_SHARED_LIB -DIO_BUILD)
		set(LTO_FLAGS "-flto")
		if(WIN32)
			set(LTO_FLAGS "${LTO_FLAGS} -Wl,-allow-multiple-definition")
			add_definitions(-D_USRDLL -D_WINDLL)
			add_library(io SHARED ${CORE_SRC} ${CORE_SRC_WINDOWS} ${NET_SRC} ${NET_SRC_WINDOWS} ${TEXTAPI_SRC} ${TLS_SRC} ${XML_SRC})
		else()
			add_library(io SHARED ${CORE_SRC} ${CORE_SRC_POSIX} ${NET_SRC} ${NET_SRC_POSIX} ${TLS_SRC} ${XML_SRC})
		endif()
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} ${LTO_FLAGS}")
		set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS} -s ${LTO_FLAGS} ${RELEASE_OPTIMIZE_FLAGS}")
		set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} ${LTO_FLAGS} ${RELEASE_OPTIMIZE_FLAGS}")
	else()
		if(WIN32)
			add_library(io STATIC ${CORE_SRC} ${CORE_SRC_WINDOWS} ${NET_SRC} ${NET_SRC_WINDOWS} ${TEXTAPI_SRC} ${TLS_SRC} ${XML_SRC})
		else()
			add_library(io STATIC ${CORE_SRC} ${CORE_SRC_POSIX} ${NET_SRC} ${NET_SRC_POSIX} ${TEXTAPI_SRC} ${TLS_SRC} ${XML_SRC})
		endif()
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} ${RELEASE_OPTIMIZE_FLAGS}" )
	endif()

	target_link_libraries(io ${EXTRA_LIBS})
	target_precompile_headers(io PRIVATE ${PROJECT_SOURCE_DIR}/src/stdafx.hpp)
	set_target_properties(io PROPERTIES LINKER_LANGUAGE CXX)

endif(CMAKE_COMPILER_IS_GNUCXX)

if(MSVC)

	if( BUILD_TYPE_RELEASE )
		add_definitions(/D "NDEBUG")
		set_property(TARGET io PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDLL$<$<CONFIG:Release>:Release>")
	else()
		set_property(TARGET io PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL$<$<CONFIG:Debug>:Debug>")
	endif()

	if(NOT NO_EXCEPTIONS)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
	endif()

	if(NO_RTTI)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GR-")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GR")
	endif()

	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} /ZI")

	set(SUPPRESS_WARNINGS "/wd4828")

	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /c /nologo /utf-8 /std:c++latest /permissive- /Zc:wchar_t ")

	if( ${CMAKE_VS_PLATFORM_TOOLSET_HOST_ARCHITECTURE} STREQUAL "x64" )
		set(CPU_SPECIFIC_FLAGS "/arch:AVX2")
	endif()

	set(OPTIMIZE_FLAGS "/O2 /sdl /Oi ${CPU_SPECIFIC_FLAGS}")
	
	
	set(Iconv_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/deps/msvc/include")
	set(Iconv_LIBRARY iconv)
	list(APPEND EXTRA_LIBS ${Iconv_LIBRARY})
	set(GNUTLS_LIBRARIES gnutls)
	list(APPEND EXTRA_LIBS ${GNUTLS_LIBRARIES})
	set(GNUTLS_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/deps/msvc/include")

	link_directories(io "${CMAKE_SOURCE_DIR}/deps/msvc/lib/${CMAKE_VS_PLATFORM_TOOLSET_HOST_ARCHITECTURE}")
	
	add_definitions(/D "_WIN32_WINNT=0x0A00" /D "_UNICODE" /D "UNICODE" /D "_CRT_SECURE_NO_WARNINGS")
	list(APPEND EXTRA_LIBS AdvAPI32)

	if(BUILD_SHARED_LIBS)
		set(SUPPRESS_WARNINGS "${SUPPRESS_WARNINGS} /wd4275 /wd4251 /wd4589")
		set(MANIFEST_SETTING "/MANIFEST /MANIFESTUAC:NO /ManifestFile:\"${LIBRARY_OUTPUT_PATH}/io.dll.intermediate.manifest\" ")
		set(UCRT "/DYNAMICBASE /DEFAULTLIB:ucrt.lib /DEFAULTLIB:vcruntime.lib /DEFAULTLIB:msvcprt.lib")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 ${SUPPRESS_WARNINGS}")
		set(CMAKE_CXX_FLAGS_RELEASE  "${CMAKE_CXX_FLAGS} ${OPTIMIZE_FLAGS} /GL /GS /GT /W3 /Gy /Zi /Gm-")
		set(CMAKE_SHARED_LINKER_FLAGS "/DLL /NXCOMPAT /SUBSYSTEM:WINDOWS ${MANIFEST_SETTING}")
		set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS} /INCREMENTAL:NO /LTCG /OPT:REF /TLBID:1 ${UCRT}")
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} /LDd /MDd")
		add_definitions(/D "IO_SHARED_LIB" /D "IO_BUILD" /D "_USRDLL" /D "_WINDLL")
		add_library(io SHARED ${PROJECT_SOURCE_DIR}/src/stdafx.cpp ${CORE_SRC} ${CORE_SRC_WINDOWS} ${NET_SRC} ${NET_SRC_WINDOWS} ${TEXTAPI_SRC} ${TLS_SRC} ${XML_SRC})
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 ${SUPPRESS_WARNINGS}")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} /O2")
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS}")
		add_library(io STATIC ${PROJECT_SOURCE_DIR}/src/stdafx.cpp ${CORE_SRC} ${CORE_SRC_WINDOWS} ${NET_SRC} ${NET_SRC_WINDOWS} ${TEXTAPI_SRC} ${TLS_SRC} ${XML_SRC})
	endif()
	
	target_precompile_headers(io PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${PROJECT_SOURCE_PATH}/stdafx.hpp>")

	target_link_libraries(io ${EXTRA_LIBS})

endif(MSVC)
