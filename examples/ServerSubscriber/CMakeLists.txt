
set(client
   serversub.cxx
   )

add_executable(ServerSubscriber ${client})
target_link_libraries(ServerSubscriber RemusProto remuscJSON)
