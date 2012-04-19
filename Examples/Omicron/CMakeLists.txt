
include(${MeshServer_SOURCE_DIR}/CMake/MeshServerRegisterWorker.cmake)
meshserver_get_includes(MeshServerWorker WorkIncludes)
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
target_link_libraries(OmicronWorker ${ZeroMQ_LIBRARIES} MeshServerWorker MeshServerCommon)
Register_Mesh_Worker(OmicronWorker 3D)
