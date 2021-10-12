cmake_minimum_required(VERSION 3.16)
project(googelTestExercise)

set(CMAKE_CXX_STANDARD 11)

# setting gtest
add_executable(googelTestExercise main.cpp)
add_subdirectory(third_party/googletest)


# Find Protobuf & Grpc
find_package(Protobuf CONFIG REQUIRED)
message(STATUS "Using protobuf ${Protobuf_VERSION}")
set(_PROTOBUF_LIBPROTOBUF protobuf::libprotobuf)
set(_REFLECTION gRPC::grpc++_reflection)

if(CMAKE_CROSSCOMPILING)
    find_program(_PROTOBUF_PROTOC protoc)
else()
    set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)
endif()

find_package(gRPC CONFIG REQUIRED)
message(STATUS "Using gRPC ${gRPC_VERSION}")
set(_GRPC_GRPCPP gRPC::grpc++)

if(CMAKE_CROSSCOMPILING)
    find_program(_GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)
else()
    set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:gRPC::grpc_cpp_plugin>)
endif()

# generate proto
## Get proto files
set (proto_name greetings)
set(GENRATED "${CMAKE_CURRENT_SOURCE_DIR}/generated")
if(NOT EXISTS ${GENERATED})
    file(MAKE_DIRECTORY ${GENRATED})
ENDIF()
include_directories(${GENRATED})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/proto)

## find proto file
get_filename_component(proto "./proto/${proto_name}.proto" ABSOLUTE)
get_filename_component(proto_dir "${proto}" DIRECTORY)

# Generate source files
set(proto_srcs "${GENRATED}/${proto_name}.pb.cc")
set(proto_hdrs "${GENRATED}/${proto_name}.pb.h")
set(grpc_srcs "${GENRATED}/${proto_name}.grpc.pb.cc")
set(grpc_hdrs "${GENRATED}/${proto_name}.grpc.pb.h")
add_custom_command(
        OUTPUT "${proto_srcs}" "${proto_hdrs}" "${grpc_srcs}" "${grpc_hdrs}"
        COMMAND
        ${_PROTOBUF_PROTOC} ARGS "--grpc_out=${GENRATED}"
        "--cpp_out=${GENRATED}" "--proto_path=${proto_dir}"
        "--plugin=protoc-gen-grpc=${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${proto}"
        DEPENDS "${proto}"
)

add_library(target STATIC
        ${proto_srcs}
        ${grpc_srcs}
        )

find_package(Threads)
target_link_libraries(googelTestExercise PRIVATE
        ${_REFLECTION}
        ${_GRPC_GRPCPP}
        ${_PROTOBUF_LIBPROTOBUF}
        target
        gtest
        Threads::Threads)



