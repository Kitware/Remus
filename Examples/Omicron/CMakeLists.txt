
include(${Remus_SOURCE_DIR}/CMake/RemusRegisterWorker.cmake)
remus_get_includes(RemusWorker WorkIncludes)
include_directories(${WorkIncludes})

set(SRCS_Client
   omicronClient.cxx
   )

set(SRCS_Workers
  OmicronWorker.cxx
  OmicronWorker.h
  workerMain.cxx
  )


add_executable(OmicronClient ${SRCS_Client} ${COMMON_SRCS})
target_link_libraries(OmicronClient ${ZeroMQ_LIBRARIES})

add_executable(OmicronWorker ${SRCS_Workers} ${COMMON_SRCS})
target_link_libraries(OmicronWorker RemusWorker RemusCommon)
Register_Mesh_Worker(OmicronWorker 3D)
