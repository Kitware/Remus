project(cJSON)

add_library(cJSON STATIC cJSON.cpp)

set_target_properties(cJSON PROPERTIES
                      POSITION_INDEPENDENT_CODE True
                      CXX_VISIBILITY_PRESET     FALSE
                      )

#if you are using this object library, you will need this source dir, so
#that you can include cJson.h
target_include_directories(cJSON INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
