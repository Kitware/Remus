project(cJSON)

add_library(cJSON STATIC cJSON.c)
if (BUILD_SHARED_LIBS)
  # Set position-independent flag when other project libs are shared.
  set_target_properties(cJSON PROPERTIES POSITION_INDEPENDENT_CODE True)
endif()

#we need to setup a user defined why of specifying a namespace around cjson
target_compile_definitions(cJson PRIVATE CJSON_NAMESPACE=${cJSON_Namespace} )


#setup install rules for json


#setup export rules for json