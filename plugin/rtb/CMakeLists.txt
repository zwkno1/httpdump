cmake_minimum_required(VERSION 2.8)

project(RTB)

option(Protobuf_USE_STATIC_LIBS ON)

find_package(Protobuf REQUIRED)

file(GLOB ProtoFiles "${CMAKE_CURRENT_SOURCE_DIR}/*.proto")
PROTOBUF_GENERATE_CPP(ProtoSources ProtoHeaders ${ProtoFiles})

include_directories(../src ${CMAKE_CURRENT_BINARY_DIR})

aux_source_directory(. SRC_LIST)

add_library(RTB SHARED ${SRC_LIST} ${ProtoSources})
target_link_libraries(RTB ${PROTOBUF_LIBRARY})
add_dependencies(RTB httpdump)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/config.json DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

