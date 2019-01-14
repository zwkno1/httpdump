cmake_minimum_required(VERSION 3.6)

project(RTB)

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
target_link_libraries(RTB ${PROTOBUF_STATIC_LIBRARIES} ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

add_dependencies(RTB protobuf_source)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/config.json DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

