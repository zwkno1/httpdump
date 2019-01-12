cmake_minimum_required(VERSION 2.8)

project(test_pb)

#set(Boost_USE_STATIC_LIBS ON)

#find_package(Boost REQUIRED COMPONENTS system)
find_package(Boost REQUIRED)

file(GLOB ProtoFiles "${CMAKE_CURRENT_SOURCE_DIR}/test.proto")

include_directories(../src ${CMAKE_CURRENT_BINARY_DIR} ${Boost_INCLUDE_DIRS} ${PROTOBUF_INCLUDE_DIRS})

aux_source_directory(. SRC_LIST)

add_library(test_pb SHARED ${SRC_LIST} ${ProtoSources})
target_link_libraries(test_pb ${PROTOBUF_LIBRARY})
add_dependencies(test_pb httpdump)

list(APPEND PY_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/test_pb2.py)
add_custom_command(OUTPUT ${PY_SOURCES}
	COMMAND ${PROTOBUF_PROTOC_EXECUTABLE} --python_out=${CMAKE_CURRENT_BINARY_DIR} --proto_path=${CMAKE_CURRENT_SOURCE_DIR} test.proto
)
add_custom_target(protobuf_sources_py ALL DEPENDS ${PY_SOURCES})
add_dependencies(protobuf_sources_py protobuf)
add_dependencies(test_pb protobuf_sources_py)

list(APPEND ProtoSources ${CMAKE_CURRENT_BINARY_DIR}/test.pb.cc)
add_custom_command(OUTPUT ${ProtoSources}
	COMMAND ${PROTOBUF_PROTOC_EXECUTABLE} --cpp_out=${CMAKE_CURRENT_BINARY_DIR} --proto_path=${CMAKE_CURRENT_SOURCE_DIR} test.proto
)
add_custom_target(protobuf_sources ALL DEPENDS ${ProtoSources})
add_dependencies(protobuf_sources protobuf)
add_dependencies(test_pb protobuf_sources)

list(APPEND TEST_FILES config.json send_request.py test.proto)
foreach(f ${TEST_FILES})
	file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/${f} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endforeach()

