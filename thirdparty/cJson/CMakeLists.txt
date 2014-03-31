project(cJSON)

add_library(cJSON STATIC cJSON.c)

set_target_properties(cJSON PROPERTIES
                      POSITION_INDEPENDENT_CODE True
                      C_VISIBILITY_PRESET     FALSE
                      )

remus_install_library(cJSON)
