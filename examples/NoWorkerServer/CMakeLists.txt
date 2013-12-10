
set(SRCS_Server
   serverMain.cxx
   )

add_executable(NoWorkerServer ${SRCS_Server})
target_link_libraries(NoWorkerServer RemusServer)
