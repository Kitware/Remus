project(cJSON)

add_library(remuscJSON STATIC cJSON.c)

set_target_properties(remuscJSON PROPERTIES
                      POSITION_INDEPENDENT_CODE True
                      C_VISIBILITY_PRESET     hidden
                      )

remus_install_library(remuscJSON)
