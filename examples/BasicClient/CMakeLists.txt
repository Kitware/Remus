
set(SRCS_Client
   clientMain.cxx
   )

include_directories( ${Remus_SOURCE_DIR} )
include_directories( ${Boost_INCLUDE_DIRS} )

add_executable(BasicClient ${SRCS_Client})
target_link_libraries(BasicClient ${ZeroMQ_LIBRARIES})
