cmake_minimum_required(VERSION 2.8)

project(test_pb)

set(Protobuf_USE_STATIC_LIBS ON)
set(Boost_USE_STATIC_LIBS ON)

find_package(Protobuf REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)

file(GLOB ProtoFiles "${CMAKE_CURRENT_SOURCE_DIR}/test.proto")
PROTOBUF_GENERATE_CPP(ProtoSources ProtoHeaders ${ProtoFiles})

include_directories(../src ${CMAKE_CURRENT_BINARY_DIR} ${Boost_INCLUDE_DIRS})

aux_source_directory(. SRC_LIST)

add_library(test_pb SHARED ${SRC_LIST} ${ProtoSources})
target_link_libraries(test_pb ${PROTOBUF_LIBRARY} ${Boost_LIBRARIES})
#set_target_properties(test_pb PROPERTIES VERSION 1.1 SOVERSION 1)
add_dependencies(test_pb httpdump)

#generate py protobuf file
# not work
#PROTOBUF_GENERATE_PYTHON(PY_SOURCES ${ProtoFiles})
list(APPEND PY_SOURCES test_pb2.py)
add_custom_command(OUTPUT ${PY_SOURCES}
	COMMAND protoc --python_out=${CMAKE_CURRENT_BINARY_DIR} --proto_path=${CMAKE_CURRENT_SOURCE_DIR} test.proto
)
add_custom_target(test_py ALL DEPENDS ${PY_SOURCES})
add_dependencies(test_pb test_py)

list(APPEND TEST_FILES config.json send_request.py test.proto)
foreach(f ${TEST_FILES})
	file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/${f} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endforeach()

