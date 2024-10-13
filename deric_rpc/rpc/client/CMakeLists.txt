message("cmake rpc client")

set(RPC_COMMON_PATH ${RPC_LIB_PATH}/common)

# add source files
aux_source_directory(src RPC_CLIENT_SRC_LIST)

# set lib properties
add_library(rpc_client_lib SHARED ${RPC_CLIENT_SRC_LIST})
set_target_properties(rpc_client_lib PROPERTIES OUTPUT_NAME "rpc_client")

# add include files
target_include_directories(rpc_client_lib PUBLIC include)
target_include_directories(rpc_client_lib PUBLIC ${COMMON_INC_PATH})

#add lib dependency
target_link_libraries(rpc_client_lib PRIVATE pthread)

add_subdirectory(${RPC_COMMON_PATH} ${OUTPUT_PATH}/rpc_common)
target_include_directories(rpc_client_lib PUBLIC ${RPC_COMMON_PATH}/include)
target_link_libraries(rpc_client_lib PUBLIC ${BEGIN_WHOLE_ARCHIVE} rpc_common_lib ${END_WHOLE_ARCHIVE})

add_subdirectory(${IO_LIB_PATH} ${OUTPUT_PATH}/io)
target_include_directories(rpc_client_lib PRIVATE ${IO_LIB_PATH}/include)
target_link_libraries(rpc_client_lib PRIVATE ${BEGIN_WHOLE_ARCHIVE} io_lib ${END_WHOLE_ARCHIVE})

add_subdirectory(${PROCESS_LIB_PATH} ${OUTPUT_PATH}/process)
target_include_directories(rpc_client_lib PUBLIC ${PROCESS_LIB_PATH}/include)
target_link_libraries(rpc_client_lib PUBLIC ${BEGIN_WHOLE_ARCHIVE} process_lib ${END_WHOLE_ARCHIVE})

add_subdirectory(${TASK_LIB_PATH} ${OUTPUT_PATH}/task)
target_include_directories(rpc_client_lib PRIVATE ${TASK_LIB_PATH}/include)
target_link_libraries(rpc_client_lib PRIVATE ${BEGIN_WHOLE_ARCHIVE} task_lib ${END_WHOLE_ARCHIVE})