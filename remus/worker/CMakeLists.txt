project(Remus_Worker)

#include the include directories from the common library
remus_get_includes(RemusCommon CommonIncludes)

set(headers
    Worker.h
    ServerConnection.h
    )

set(worker_srcs
   Worker.cxx
   )

add_library(RemusWorker ${worker_srcs} ${headers})
set(RemusWorker_Libraries ${ZeroMQ_LIBRARIES} ${Boost_LIBRARIES})
if(UNIX)
  #we must link against pthreads so that we compile, and properly
  #export the pthread link line
  list(APPEND RemusWorker_Libraries ${CMAKE_THREAD_LIBS_INIT} )
endif()
target_link_libraries(RemusWorker ${RemusWorker_Libraries})

remus_set_includes(RemusWorker ${CommonIncludes})

#create the export header symbol exporting
remus_export_header(RemusWorker WorkerExports.h)

#install the library
remus_install_library(RemusWorker)

#install the headers
remus_public_headers(${headers})

#setup the exports for the library when used from a build tree
export(PACKAGE RemusWorker)
export(TARGETS RemusWorker RemusCommon sysTools FILE RemusWorker-exports.cmake)
