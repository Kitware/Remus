
set(SRCS_Client
   clientMain.cxx
   )

include_directories( ${Remus_SOURCE_DIR} )

add_executable(BasicClient ${SRCS_Client} ${COMMON_SRCS})
target_link_libraries(BasicClient ${ZeroMQ_LIBRARIES})
