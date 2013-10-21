
remus_get_includes(RemusServer WorkIncludes)
include_directories(${WorkIncludes})

set(SRCS_Server
   serverMain.cxx
   )

add_executable(NoWorkerServer ${SRCS_Server} ${COMMON_SRCS})
target_link_libraries(NoWorkerServer RemusServer)
