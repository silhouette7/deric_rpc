cmake_minimum_required(VERSION 3.5)

project(test_task)

set(RPC_SDK_PATH ../../deric_rpc)
set(TASK_LIB_PATH ${RPC_SDK_PATH}/task)
set(OUTPUT_PATH ./output)

set(BEGIN_WHOLE_ARCHIVE -Wl,--whole-archive)
set(END_WHOLE_ARCHIVE -Wl,--no-whole-archive)

aux_source_directory(. SRC_LIST)

add_compile_options(-std=c++17)

INCLUDE_DIRECTORIES(${TASK_LIB_PATH}/include
                    ${RPC_SDK_PATH}/common)

add_executable(test_task ${SRC_LIST})

add_subdirectory(${TASK_LIB_PATH} ${OUTPUT_PATH}/task)
target_link_libraries(test_task PUBLIC ${BEGIN_WHOLE_ARCHIVE} task_lib ${END_WHOLE_ARCHIVE})
target_link_libraries(test_task PRIVATE pthread)