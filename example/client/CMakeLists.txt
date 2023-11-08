cmake_minimum_required(VERSION 3.5)

project(demo_client)

set(RPC_SDK_PATH ../../deric_rpc)

aux_source_directory(. CLIENT_SRC_LIST)

add_compile_options(-std=c++14)

INCLUDE_DIRECTORIES(../server/api
                    ${RPC_SDK_PATH}/rpc/client/include
                    ${RPC_SDK_PATH}/process/include
                    ${RPC_SDK_PATH}/config/include
                    ${RPC_SDK_PATH}/serial
                    ${RPC_SDK_PATH}/common)

add_executable(client ${CLIENT_SRC_LIST})

find_library(RPC_CLIENT_LIB librpc_client.so HINT ${RPC_SDK_PATH}/output/client)

target_link_libraries(client ${RPC_CLIENT_LIB})