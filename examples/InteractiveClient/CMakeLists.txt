
set(client_two
  interactiveClient.cxx
   )

include_directories( ${Remus_SOURCE_DIR} )

add_executable(InteractiveClient ${client_two})
target_link_libraries(InteractiveClient ${ZeroMQ_LIBRARIES})
