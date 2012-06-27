
set(SRCS_Client
   clientMain.cxx
   )

add_executable(BasicClient ${SRCS_Client} ${COMMON_SRCS})
target_link_libraries(BasicClient ${ZeroMQ_LIBRARIES})
