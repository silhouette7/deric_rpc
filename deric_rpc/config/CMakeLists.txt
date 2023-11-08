message("cmake config")

aux_source_directory(src CONFIG_SRC_PATH)

add_library(config_lib STATIC ${CONFIG_SRC_PATH})

set_target_properties(config_lib PROPERTIES OUTPUT_NAME "config")

target_include_directories(config_lib PUBLIC include)