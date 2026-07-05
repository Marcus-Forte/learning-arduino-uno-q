cmake_minimum_required(VERSION 3.16)

project(learning_arduino_q1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(findGRPC)
include(generate_proto)
include(arduino_cli_targets)
include(format)

add_arduino_cli_compile_target(src_mcu/rpc)
add_arduino_cli_compile_target(src_mcu/hello)

find_package(msgpack-cxx REQUIRED)

generate_proto(${CMAKE_CURRENT_SOURCE_DIR}/proto/mcu.proto)

add_library(mcu_rpc 
    ${CMAKE_CURRENT_BINARY_DIR}/mcu.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/mcu.grpc.pb.cc)
target_include_directories(mcu_rpc PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(mcu_rpc PUBLIC ${_PROTOBUF_LIBPROTOBUF} ${_GRPC_GRPCPP})

add_executable(hello
    src/hello/main.cc
)
install(TARGETS hello DESTINATION "${CMAKE_INSTALL_BINDIR}")

add_executable(bridge
    src/bridge/main.cc
    src/bridge/bridge.cc
)
target_link_libraries(bridge mcu_rpc msgpack-cxx)
install(TARGETS bridge DESTINATION "${CMAKE_INSTALL_BINDIR}")