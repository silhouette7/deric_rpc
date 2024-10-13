message("cmake rpc server")

set(RPC_COMMON_PATH ${RPC_LIB_PATH}/common)

# add source files
aux_source_directory(src RPC_SERVER_SRC_LIST)

# set lib properties
add_library(rpc_server_lib SHARED ${RPC_SERVER_SRC_LIST})
set_target_properties(rpc_server_lib PROPERTIES OUTPUT_NAME "rpc_server")

# add include files
target_include_directories(rpc_server_lib PUBLIC include)
target_include_directories(rpc_server_lib PUBLIC ${COMMON_INC_PATH})

#add lib dependency
target_link_libraries(rpc_server_lib PRIVATE pthread)

add_subdirectory(${RPC_COMMON_PATH} ${OUTPUT_PATH}/rpc_common)
target_include_directories(rpc_server_lib PUBLIC ${RPC_COMMON_PATH}/include)
target_link_libraries(rpc_server_lib PUBLIC ${BEGIN_WHOLE_ARCHIVE} rpc_common_lib ${END_WHOLE_ARCHIVE})

add_subdirectory(${IO_LIB_PATH} ${OUTPUT_PATH}/io)
target_include_directories(rpc_server_lib PRIVATE ${IO_LIB_PATH}/include)
target_link_libraries(rpc_server_lib PRIVATE ${BEGIN_WHOLE_ARCHIVE} io_lib ${END_WHOLE_ARCHIVE})

add_subdirectory(${PROCESS_LIB_PATH} ${OUTPUT_PATH}/process)
target_include_directories(rpc_server_lib PUBLIC ${PROCESS_LIB_PATH}/include)
target_link_libraries(rpc_server_lib PUBLIC ${BEGIN_WHOLE_ARCHIVE} process_lib ${END_WHOLE_ARCHIVE})

add_subdirectory(${TASK_LIB_PATH} ${OUTPUT_PATH}/task)
target_include_directories(rpc_server_lib PRIVATE ${TASK_LIB_PATH}/include)
target_link_libraries(rpc_server_lib PRIVATE ${BEGIN_WHOLE_ARCHIVE} task_lib ${END_WHOLE_ARCHIVE})