
set(client_two
  interactiveClient.cxx
   )

add_executable(InteractiveClient ${client_two} ${COMMON_SRCS})
target_link_libraries(InteractiveClient ${ZeroMQ_LIBRARIES})
