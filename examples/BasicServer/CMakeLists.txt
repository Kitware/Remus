
set(SRCS_Server
   serverMain.cxx
   )

add_executable(BasicServer ${SRCS_Server} ${COMMON_SRCS})
target_link_libraries(BasicServer RemusServer)
