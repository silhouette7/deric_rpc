aux_source_directory(./src TASK_SRC_PATH)

add_library(task_lib STATIC ${TASK_SRC_PATH})

set_target_properties(task_lib PROPERTIES OUTPUT_NAME "task")

target_include_directories(task_lib PUBLIC ./include)
target_include_directories(task_lib PUBLIC ${COMMON_INC_PATH})