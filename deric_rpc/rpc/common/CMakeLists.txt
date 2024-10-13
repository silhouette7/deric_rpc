message("cmake rpc common")

# add source files
aux_source_directory(src RPC_COMMON_SRC_LIST)

# set lib properties
add_library(rpc_common_lib STATIC ${RPC_COMMON_SRC_LIST})
set_target_properties(rpc_common_lib PROPERTIES OUTPUT_NAME "rpc_common")

# add include files
target_include_directories(rpc_common_lib PUBLIC include)
target_include_directories(rpc_common_lib PUBLIC ${COMMON_INC_PATH})