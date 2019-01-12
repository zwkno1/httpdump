cmake_minimum_required(VERSION 2.8)

project(RTB)

#set(Boost_USE_STATIC_LIBS ON)

find_package(Boost REQUIRED)

file(GLOB ProtoFiles "${CMAKE_CURRENT_SOURCE_DIR}/*.proto")
list(APPEND ProtoSources BAIDU.pb.cc RTB.pb.cc TENCENT.pb.cc WIFI.pb.cc)
add_custom_command(OUTPUT ${ProtoSources}
	COMMAND ${PROTOBUF_PROTOC_EXECUTABLE} --cpp_out=${CMAKE_CURRENT_BINARY_DIR} --proto_path=${CMAKE_CURRENT_SOURCE_DIR} ${ProtoFiles}
)
add_custom_target(protobuf_source ALL DEPENDS ${ProtoSources})
add_dependencies(protobuf_source protobuf)

include_directories(../src ${CMAKE_CURRENT_BINARY_DIR} ${Boost_INCLUDE_DIRS} ${PROTOBUF_INCLUDE_DIRS})

aux_source_directory(. SRC_LIST)

add_library(RTB SHARED ${SRC_LIST} ${ProtoSources})
target_link_libraries(RTB ${PROTOBUF_LIBRARY})

add_dependencies(RTB protobuf_source)
add_dependencies(RTB httpdump)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/config.json DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

