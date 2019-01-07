
include(ExternalProject)

ExternalProject_Add(libtins
	PREFIX            "external/libtins"
	URL               "https://github.com/mfontanini/libtins/archive/v4.1.tar.gz"
	URL_MD5           1177f150a5bf375eb990a449b8b8f804
	#BUILD_IN_SOURCE 1
	#CONFIGURE_COMMAND cmake ${CMAKE_BINARY_DIR}/external/libtins/src/libtins/ -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/external/libtins -DLIBTINS_BUILD_SHARED=0
	CONFIGURE_COMMAND cmake ${CMAKE_BINARY_DIR}/external/libtins/src/libtins/ -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/external/libtins -DLIBTINS_BUILD_SHARED=0
    BUILD_COMMAND     make
    INSTALL_COMMAND   make install
    )

#add_library(libtins STATIC IMPORTED)
#include(${CMAKE_BINARY_DIR}/external/libtins/CMake/libtinsConfig.cmake )
#include(${CMAKE_BINARY_DIR}/external/libtins/CMake/libtinsConfigVersion.cmake )
#add_library(tins STATIC IMPORTED)
set(LIBTINS_FOUND "TRUE")
set(LIBTINS_INCLUDE_DIRS "${CMAKE_BINARY_DIR}/external/libtins/include")
set(LIBTINS_LIBRARIES "${CMAKE_BINARY_DIR}/external/libtins/lib/libtins.a")

