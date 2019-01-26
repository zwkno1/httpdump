
set (DOWNLOAD_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/downloads" CACHE PATH "Location where external projects will be downloaded.")
option(ENABLE_POSITION_INDEPENDENT_CODE "Enable PIE support" ON)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/external/libtins.cmake)
#include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/external/zlib.cmake)
#include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/external/protobuf.cmake)
