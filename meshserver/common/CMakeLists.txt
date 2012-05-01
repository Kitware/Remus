project(MeshServer_Common)

#include the build directory for the export header
set(MeshServer_Common_INCLUDE_DIRS
  ${ZeroMQ_INCLUDE_DIR}
  ${Boost_INCLUDE_DIRS}
  )

set(headers
  ExecuteProcess.h
  JobDetails.h
  JobResult.h
  JobStatus.h
  meshServerGlobals.h
  zmqHelper.h
  )

set(common_srcs
  ExecuteProcess.cxx
  )

add_library(MeshServerCommon ${common_srcs} ${headers})
target_link_libraries(MeshServerCommon ${ZeroMQ_LIBRARIES} sysTools)

generate_export_header(MeshServerCommon EXPORT_FILE_NAME CommonExports.h)
meshserver_set_includes(MeshServerCommon ${MeshServer_Common_INCLUDE_DIRS})

#install the library
meshserver_install_library(MeshServerCommon)

#install the headers
meshserver_public_headers(${headers})
