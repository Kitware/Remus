

get_project_properties(boost boost_args)
get_project_properties(zeroMQ zero_args)

message(STATUS "boost args are: ${boost_args}")
message(STATUS "zeroMQ args are: ${zero_args}")

add_external_project(remus
  DEPENDS boost zeroMQ
  CMAKE_ARGS
   -DRemus_ENABLE_EXAMPLES:BOOL=OFF
   -DRemus_NO_SYSTEM_BOOST:BOOL=ON
    ${boost_args}
    ${zero_args}
  )
