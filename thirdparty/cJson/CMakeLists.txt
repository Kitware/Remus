project(remuscJSON)

add_library(remuscJSON STATIC cJSON.cxx)

set_target_properties(remuscJSON PROPERTIES
                      POSITION_INDEPENDENT_CODE True
                      CXX_VISIBILITY_PRESET     hidden
                      )

remus_install_library(remuscJSON)
