remus_get_includes(RemusCommon CommonIncludes)
include_directories(${CommonIncludes})

set(SRCS_Client
   clientMain.cxx
   )

add_executable(BasicClient ${SRCS_Client} ${COMMON_SRCS})
target_link_libraries(BasicClient ${ZeroMQ_LIBRARIES})
