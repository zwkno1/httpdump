cmake_minimum_required(VERSION 2.8)

project (httpdump)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

include(${CMAKE_CURRENT_SOURCE_DIR}/external/external.cmake)

find_package(Threads)
find_package(Boost REQUIRED COMPONENTS system)
find_package(PCAP REQUIRED)

include_directories(${Boost_INCLUDE_DIRS} ${PCAP_INCLUDE_DIRS})

set(SOURCE_FILES main.cpp)

add_executable(httpdump ${SOURCE_FILES})

target_link_libraries(httpdump  ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES} ${PCAP_LIBRARIES})

