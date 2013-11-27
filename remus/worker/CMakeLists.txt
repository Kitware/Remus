project(Remus_Worker)

set(headers
    Job.h
    JobResult.h
    JobStatus.h
    ServerConnection.h
    Worker.h
    )

set(worker_srcs
   Worker.cxx
   )

add_library(RemusWorker ${worker_srcs} ${headers})

#need to link to the threading libraries to make sure threading is properly
#set up
target_link_libraries(RemusWorker
                      LINK_PUBLIC RemusCommon ${Boost_LIBRARIES}
                      LINK_PRIVATE ${CMAKE_THREAD_LIBS_INIT})

#create the export header symbol exporting
remus_export_header(RemusWorker WorkerExports.h)

#install the library
remus_install_library(RemusWorker)

#install the headers
remus_public_headers(${headers})

#setup the exports for the library when used from a build tree
export(PACKAGE RemusWorker)
export(TARGETS RemusWorker RemusCommon sysTools FILE RemusWorker-exports.cmake)

if(Remus_ENABLE_TESTING)
  add_subdirectory(testing)
endif()
