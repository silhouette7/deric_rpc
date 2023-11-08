aux_source_directory(src PROCESS_SRC_PATH)

add_library(process_lib STATIC ${PROCESS_SRC_PATH})

set_target_properties(process_lib PROPERTIES OUTPUT_NAME "process")

target_include_directories(process_lib PUBLIC include)
target_include_directories(process_lib PRIVATE ${IO_LIB_PATH}/include)