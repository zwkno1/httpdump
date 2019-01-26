cmake_minimum_required(VERSION 2.8)

project(test_pb)

file(GLOB ProtoFiles "${CMAKE_CURRENT_SOURCE_DIR}/test.proto")
protobuf_generate_cpp(ProtoSources ProtoHeaders ${ProtoFiles})
protobuf_generate_python(PROTO_PY ${ProtoFiles})
add_custom_target(proto_py ALL DEPENDS ${PROTO_PY})

#list(APPEND PY_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/test_pb2.py)
#add_custom_command(OUTPUT ${PY_SOURCES}
#	COMMAND ${PROTOBUF_PROTOC_EXECUTABLE} --python_out=${CMAKE_CURRENT_BINARY_DIR} --proto_path=${CMAKE_CURRENT_SOURCE_DIR} test.proto
#)
#
#list(APPEND ProtoSources ${CMAKE_CURRENT_BINARY_DIR}/test.pb.cc)
#add_custom_command(OUTPUT ${ProtoSources}
#	COMMAND ${PROTOBUF_PROTOC_EXECUTABLE} --cpp_out=${CMAKE_CURRENT_BINARY_DIR} --proto_path=${CMAKE_CURRENT_SOURCE_DIR} test.proto
#)
#add_custom_target(protobuf_sources ALL DEPENDS ${ProtoSources})
#add_dependencies(protobuf_sources protobuf)

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${Protobuf_INCLUDE_DIRS})

aux_source_directory(. SRC_LIST)
add_library(test_pb SHARED ${SRC_LIST} ${ProtoHeaders} ${ProtoSources})
target_link_libraries(test_pb ${Protobuf_LIBRARIES} ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
add_dependencies(test_pb proto_py)
#add_dependencies(test_pb protobuf_sources)

list(APPEND TEST_FILES config.json send_request.py test.proto)
foreach(f ${TEST_FILES})
	file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/${f} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endforeach()

