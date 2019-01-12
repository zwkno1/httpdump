
set (DOWNLOAD_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/downloads" CACHE PATH "Location where external projects will be downloaded.")
set(ENABLE_POSITION_INDEPENDENT_CODE ON)

#include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/external/libnl.cmake)
#include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/external/libpcap.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/external/libtins.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/external/zlib.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/external/protobuf.cmake)
