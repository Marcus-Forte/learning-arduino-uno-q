cmake_minimum_required(VERSION 3.16)

project(learning_arduino_q1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# generate_proto(${CMAKE_CURRENT_SOURCE_DIR}/proto/mcu.proto ${CMAKE_SOURCE_DIR}/third_party/msensor/proto)

include(findGRPC)
include(generate_proto)
include(arduino_cli_targets)

find_package(msgpack-cxx REQUIRED)

generate_proto(${CMAKE_CURRENT_SOURCE_DIR}/proto/mcu.proto)

add_library(mcu_rpc 
    ${CMAKE_CURRENT_BINARY_DIR}/mcu.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/mcu.grpc.pb.cc)
target_include_directories(mcu_rpc PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(mcu_rpc PUBLIC ${_PROTOBUF_LIBPROTOBUF} ${_GRPC_GRPCPP})

add_executable(learning_arduino_q1
    src/main.cc
    src/bridge.cc
)
target_link_libraries(learning_arduino_q1 mcu_rpc msgpack-cxx)