message("cmake io")

aux_source_directory(src IO_SRC_PATH)

add_library(io_lib STATIC ${IO_SRC_PATH})

set_target_properties(io_lib PROPERTIES OUTPUT_NAME "io")

target_include_directories(io_lib PUBLIC include)
target_include_directories(io_lib PRIVATE ${TASK_LIB_PATH}/include)
target_include_directories(io_lib PUBLIC ${COMMON_INC_PATH})