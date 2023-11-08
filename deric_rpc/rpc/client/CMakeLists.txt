message("cmake rpc client")

# add source files
aux_source_directory(src RPC_CLIENT_SRC_LIST)

# set lib properties
add_library(rpc_client_lib SHARED ${RPC_CLIENT_SRC_LIST})
set_target_properties(rpc_client_lib PROPERTIES OUTPUT_NAME "rpc_client")
target_link_libraries(rpc_client_lib PRIVATE pthread)

# add include files
INCLUDE_DIRECTORIES(${COMMON_INC_PATH})

# add hpp dependency
target_include_directories(rpc_client_lib PUBLIC include)
target_include_directories(rpc_client_lib PRIVATE ${SERIAL_INC_PATH})

# add lib denpendency
add_subdirectory(${CONFIG_LIB_PATH} ${OUTPUT_PATH}/config)
target_include_directories(rpc_client_lib PUBLIC ${CONFIG_LIB_PATH}/include)
target_link_libraries(rpc_client_lib PUBLIC ${BEGIN_WHOLE_ARCHIVE} config_lib ${END_WHOLE_ARCHIVE})

add_subdirectory(${IO_LIB_PATH} ${OUTPUT_PATH}/io)
target_include_directories(rpc_client_lib PRIVATE ${IO_LIB_PATH}/include)
target_link_libraries(rpc_client_lib PRIVATE ${BEGIN_WHOLE_ARCHIVE} io_lib ${END_WHOLE_ARCHIVE})

add_subdirectory(${PROCESS_LIB_PATH} ${OUTPUT_PATH}/process)
target_include_directories(rpc_client_lib PUBLIC ${PROCESS_LIB_PATH}/include)
target_link_libraries(rpc_client_lib PUBLIC ${BEGIN_WHOLE_ARCHIVE} process_lib ${END_WHOLE_ARCHIVE})