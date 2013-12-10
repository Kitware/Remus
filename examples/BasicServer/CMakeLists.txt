
set(SRCS_Server
   serverMain.cxx
   )

add_executable(BasicServer ${SRCS_Server})
target_link_libraries(BasicServer RemusServer)
