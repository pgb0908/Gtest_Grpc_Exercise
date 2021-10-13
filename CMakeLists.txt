cmake_minimum_required(VERSION 3.16)
project(googelTestExercise)

set(CMAKE_CXX_STANDARD 11)

# setting gtest
add_subdirectory(third_party/googletest)

# find Threads
find_package(Threads)

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

add_library(greeting_proto STATIC
        ${proto_srcs}
        ${grpc_srcs}
        )

############# test1 #########################
add_executable(basic_grpc_test basic_grpc_test.cpp)
target_link_libraries(basic_grpc_test PUBLIC
        ${_REFLECTION}
        ${_GRPC_GRPCPP}
        ${_PROTOBUF_LIBPROTOBUF}
        greeting_proto
        gtest
        Threads::Threads
        )
######################################

## otel linking
option(WITH_OTLP "we use gRPC" on)
option(CMAKE_POSITION_INDEPENDENT_CODE "" on)
option(BUILD_TESTING "we skip test" off)
option(WITH_EXAMPLES "we skip example" off)

include_directories(
        ${CMAKE_BINARY_DIR}/generated/third_party/opentelemetry-proto)
include_directories(third_party/opentelemetry-cpp/exporters/otlp/include)
include_directories(third_party/opentelemetry-cpp/exporters/ostream/include)

# setting otel
add_subdirectory(third_party/opentelemetry-cpp)

add_executable(otel_grpc_test otel_grpc_test.cpp)
target_link_libraries(otel_grpc_test PUBLIC
        opentelemetry_api
        opentelemetry_trace
        opentelemetry_exporter_ostream_span
        opentelemetry_exporter_otlp_grpc
        ${_GRPC_GRPCPP}
        gtest
        Threads::Threads
        )
