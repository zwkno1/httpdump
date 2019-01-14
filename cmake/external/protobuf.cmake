include (ExternalProject)

# enable choose protobuf versions
#SET(PROTOBUF_VERSION "3.6.1" CACHE STRING "Protobuf version")
#SET_PROPERTY(CACHE PROTOBUF_VERSION PROPERTY STRINGS "3.4.0" "3.5.0" "3.6.1") 
#if(${PROTOBUF_VERSION} STREQUAL "3.5.1")
	set(PROTOBUF_TAG v3.6.1)
#elseif(${PROTOBUF_VERSION} STREQUAL "3.5.0")
#	set(PROTOBUF_TAG 2761122b810fe8861004ae785cc3ab39f384d342)
#elseif(${PROTOBUF_VERSION} STREQUAL "3.4.0")
#	set(PROTOBUF_TAG b04e5cba356212e4e8c66c61bbe0c3a20537c5b9)
#endif()
#set(PROTOBUF_URL https://github.com/google/protobuf.git)
set(PROTOBUF_URL https://github.com/protocolbuffers/protobuf/archive/${PROTOBUF_TAG}.tar.gz)
set(PROTOBUF_MD5 e8ce2659ea4f5df1a1e0dbd107dd61d9)

set(PROTOBUF_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR}/external/protobuf/src/protobuf/src)

if(WIN32)
	if(${CMAKE_GENERATOR} MATCHES "Visual Studio.*")
		set(protobuf_STATIC_LIBRARIES 
			debug ${CMAKE_CURRENT_BINARY_DIR}/external/protobuf/src/protobuf/$(Configuration)/libprotobufd.lib
			optimized ${CMAKE_CURRENT_BINARY_DIR}/external/protobuf/src/protobuf/$(Configuration)/libprotobuf.lib)
		set(PROTOBUF_PROTOC_EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/external/protobuf/src/protobuf/$(Configuration)/protoc.exe)
	else()
		if(CMAKE_BUILD_TYPE EQUAL Debug)
			set(protobuf_STATIC_LIBRARIES
				${CMAKE_CURRENT_BINARY_DIR}/external/protobuf/src/protobuf/libprotobufd.lib)
		else()
			set(protobuf_STATIC_LIBRARIES
				${CMAKE_CURRENT_BINARY_DIR}/external/protobuf/src/protobuf/libprotobuf.lib)
		endif()
		set(PROTOBUF_PROTOC_EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/external/protobuf/src/protobuf/protoc.exe)
	endif()

	# This section is to make sure CONFIGURE_COMMAND use the same generator settings
	set(PROTOBUF_GENERATOR_PLATFORM)
	if (CMAKE_GENERATOR_PLATFORM)
		set(PROTOBUF_GENERATOR_PLATFORM -A ${CMAKE_GENERATOR_PLATFORM})
	endif()
	set(PROTOBUF_GENERATOR_TOOLSET)
	if (CMAKE_GENERATOR_TOOLSET)
		set(PROTOBUF_GENERATOR_TOOLSET -T ${CMAKE_GENERATOR_TOOLSET})
	endif()
	set(PROTOBUF_ADDITIONAL_CMAKE_OPTIONS	-Dprotobuf_MSVC_STATIC_RUNTIME:BOOL=OFF
		-G${CMAKE_GENERATOR} ${PROTOBUF_GENERATOR_PLATFORM} ${PROTOBUF_GENERATOR_TOOLSET})
	# End of section
else()
	set(PROTOBUF_STATIC_LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/external/protobuf/src/protobuf/libprotobuf.a)
	set(PROTOBUF_PROTOC_EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/external/protobuf/src/protobuf/protoc)
endif()

ExternalProject_Add(protobuf
	PREFIX external/protobuf
	DEPENDS zlib
	#GIT_REPOSITORY ${PROTOBUF_URL}
	#GIT_TAG ${PROTOBUF_TAG}
	URL ${PROTOBUF_URL}
	URL_MD5 ${PROTOBUF_MD5}
	DOWNLOAD_DIR "${DOWNLOAD_LOCATION}"
	BUILD_IN_SOURCE 1
	BUILD_BYPRODUCTS ${PROTOBUF_PROTOC_EXECUTABLE} ${protobuf_STATIC_LIBRARIES}
	SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/external/protobuf/src/protobuf
	# SOURCE_SUBDIR cmake/ # Requires CMake 3.7, this will allow removal of CONFIGURE_COMMAND
	# CONFIGURE_COMMAND resets some settings made in CMAKE_CACHE_ARGS and the generator used
	CONFIGURE_COMMAND ${CMAKE_COMMAND} cmake/
	-DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=${ENABLE_POSITION_INDEPENDENT_CODE}
	-DCMAKE_BUILD_TYPE:STRING=Release
	-DCMAKE_VERBOSE_MAKEFILE:BOOL=OFF
	-Dprotobuf_BUILD_TESTS:BOOL=OFF
	-DZLIB_ROOT=${ZLIB_INSTALL}
	${PROTOBUF_ADDITIONAL_CMAKE_OPTIONS}
	INSTALL_COMMAND ""
	CMAKE_CACHE_ARGS
	-DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=${ENABLE_POSITION_INDEPENDENT_CODE}
	-DCMAKE_BUILD_TYPE:STRING=Release
	-DCMAKE_VERBOSE_MAKEFILE:BOOL=OFF
	-Dprotobuf_BUILD_TESTS:BOOL=OFF
	-Dprotobuf_MSVC_STATIC_RUNTIME:BOOL=OFF
	-DZLIB_ROOT:STRING=${ZLIB_INSTALL}
	)
