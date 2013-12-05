
set(client_two
  interactiveClient.cxx
   )

add_executable(InteractiveClient ${client_two})
target_link_libraries(InteractiveClient RemusClient)
