cmake_minimum_required(VERSION 3.5)

project(demo_server)

set(RPC_SDK_PATH ../../../deric_rpc)

aux_source_directory(. SERVER_SOURCE_LIST)

add_compile_options(-std=c++17)

INCLUDE_DIRECTORIES(${RPC_SDK_PATH}/rpc/server/include
                    ${RPC_SDK_PATH}/rpc/common/include
                    ${RPC_SDK_PATH}/process/include
                    ${RPC_SDK_PATH}/common)

add_executable(server ${SERVER_SOURCE_LIST})

find_library(RPC_SERVER_LIB librpc_server.so HINT ${RPC_SDK_PATH}/output/server)

target_link_libraries(server ${RPC_SERVER_LIB})