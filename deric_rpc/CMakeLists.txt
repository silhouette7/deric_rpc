cmake_minimum_required(VERSION 3.5)

project(rpc_lib)

set(COMMON_INC_PATH ${PROJECT_SOURCE_DIR}/common)
set(IO_LIB_PATH ${PROJECT_SOURCE_DIR}/io)
set(PROCESS_LIB_PATH ${PROJECT_SOURCE_DIR}/process)
set(RPC_LIB_PATH ${PROJECT_SOURCE_DIR}/rpc)
set(TASK_LIB_PATH ${PROJECT_SOURCE_DIR}/task)
set(OUTPUT_PATH ${PROJECT_SOURCE_DIR}/output)

set(BEGIN_WHOLE_ARCHIVE -Wl,--whole-archive)
set(END_WHOLE_ARCHIVE -Wl,--no-whole-archive)

option(SERVER_BUILD "build server lib" OFF)
option(CLIENT_BUILD "build client lib" OFF)

add_compile_options(-std=c++17 -fPIC )
add_definitions(-DDEBUG_ENABLE)

if(SERVER_BUILD)
    add_subdirectory(${RPC_LIB_PATH}/server ${OUTPUT_PATH}/server)
endif()

if(CLIENT_BUILD)
    add_subdirectory(${RPC_LIB_PATH}/client ${OUTPUT_PATH}/client)
endif()