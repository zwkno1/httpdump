cmake_minimum_required(VERSION 2.8)

project(protobuf_test)

find_package(Protobuf REQUIRED)

file(GLOB ProtoFiles "${CMAKE_CURRENT_SOURCE_DIR}/test.proto")
PROTOBUF_GENERATE_CPP(ProtoSources ProtoHeaders ${ProtoFiles})
PROTOBUF_GENERATE_PYTHON(PY_SOURCES ${ProtoFiles})

include_directories(../src ${CMAKE_CURRENT_BINARY_DIR})

aux_source_directory(. SRC_LIST)

add_library(protobuf_test SHARED ${SRC_LIST} ${ProtoSources})
target_link_libraries(protobuf_test ${PROTOBUF_LIBRARY})


#set_target_properties(protobuf_test PROPERTIES VERSION 1.1 SOVERSION 1)

