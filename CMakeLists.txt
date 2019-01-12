cmake_minimum_required(VERSION 2.8)

project (httpdump)

option(BUILD_PLUGIN ON)
set(Boost_USE_STATIC_LIBS ON)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/external.cmake)

find_package(Threads)
find_package(Boost REQUIRED COMPONENTS system filesystem program_options)
find_package(PCAP REQUIRED)

include_directories(${LIBTINS_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS} ./src)

set(SOURCE_FILES src/main.cpp
    )

add_executable(httpdump ${SOURCE_FILES})
add_dependencies(httpdump libtins)
#add_dependencies(httpdump libpcap)

target_link_libraries(httpdump ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES} ${LIBTINS_LIBRARIES} ${PCAP_LIBRARIES} ${CMAKE_DL_LIBS})

if(BUILD_PLUGIN)
	add_subdirectory(plugin/test_pb)
	add_subdirectory(plugin/rtb)
endif()

