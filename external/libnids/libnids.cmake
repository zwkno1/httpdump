
include(ExternalProject)

ExternalProject_Add(libnids
	PREFIX            "external/libnids"
	#URL               "${CMAKE_CURRENT_LIST_DIR}/libnids-1.24.tar.gz"
	URL               "https://managedway.dl.sourceforge.net/project/libnids/libnids/1.24/libnids-1.24.tar.gz"
	URL_MD5           72d37c79c85615ffe158aa524d649610
    #DOWNLOAD_COMMAND ${EXTERNAL_CURRENT_DOWNLOAD_COMMAND}
	BUILD_IN_SOURCE 1
	#CONFIGURE_COMMAND ${CMAKE_BINARY_DIR}/external/libnids/src/libnids/configure --prefix=${CMAKE_BINARY_DIR}/external/libnids
	CONFIGURE_COMMAND ./configure --prefix=${CMAKE_BINARY_DIR}/external/libnids
    BUILD_COMMAND     make
    INSTALL_COMMAND   make install
    )

add_library(nids STATIC IMPORTED)
set(NIDS_FOUND "TRUE")
set(NIDS_INCLUDE_DIRS "${CMAKE_BINARY_DIR}/external/libnids/include")
set(NIDS_LIBRARIES "${CMAKE_BINARY_DIR}/external/libnids/lib/libnids.a")
