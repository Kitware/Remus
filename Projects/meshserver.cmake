

get_project_properties(boost boost_args)
get_project_properties(zeroMQ zero_args)

message(STATUS "boost args are: ${boost_args}")
message(STATUS "zeroMQ args are: ${zero_args}")

add_external_project(meshserver
  DEPENDS boost zeroMQ
  CMAKE_ARGS
    ${boost_args}
    ${zero_args}
  )
