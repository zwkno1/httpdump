cmake_minimum_required(VERSION 2.8)

project(protobuf_test)

find_package(Protobuf REQUIRED)

file(GLOB ProtoFiles "${CMAKE_CURRENT_SOURCE_DIR}/test.proto")
PROTOBUF_GENERATE_CPP(ProtoSources ProtoHeaders ${ProtoFiles})

include_directories(../src ${CMAKE_CURRENT_BINARY_DIR})

aux_source_directory(. SRC_LIST)

add_library(protobuf_test SHARED ${SRC_LIST} ${ProtoSources})
target_link_libraries(protobuf_test ${PROTOBUF_LIBRARY})
#set_target_properties(protobuf_test PROPERTIES VERSION 1.1 SOVERSION 1)
add_dependencies(protobuf_test httpdump)

#generate py protobuf file
# not work
#PROTOBUF_GENERATE_PYTHON(PY_SOURCES ${ProtoFiles})
list(APPEND PY_SOURCES test_pb2.py)
add_custom_command(OUTPUT ${PY_SOURCES}
	COMMAND protoc --python_out=${CMAKE_CURRENT_BINARY_DIR} --proto_path=${CMAKE_CURRENT_SOURCE_DIR} test.proto
)
add_custom_target(test_py ALL DEPENDS ${PY_SOURCES})
add_dependencies(protobuf_test test_py)

list(APPEND TEST_FILES plugin.json send_request.py test.proto)
foreach(f ${TEST_FILES})
	file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/${f} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endforeach()

