project(Remus_Worker)

#We need to verify if we have the auxv header, so that we can figure out
#the best way to do current executable directory lookup
include (CheckIncludeFiles)
check_include_files("sys/auxv.h"    REMUS_HAVE_AUXV_H)

add_subdirectory(detail)

set(headers
    Job.h
    ServerConnection.h
    Worker.h
    )

set(worker_srcs
   ServerConnection.cxx
   Worker.cxx
   detail/JobQueue.cxx
   detail/MessageRouter.cxx
   )

add_library(RemusWorker ${worker_srcs} ${headers})

#need to link to the threading libraries to make sure threading is properly
#set up
target_link_libraries(RemusWorker
                      LINK_PUBLIC RemusProto
                      LINK_PRIVATE ${Boost_LIBRARIES}
                                   ${CMAKE_THREAD_LIBS_INIT} )

#disable checked iterators in RemusWorker
if(MSVC)
  target_compile_definitions(RemusWorker PRIVATE _SCL_SECURE_NO_WARNINGS)
endif()

#tell the worker code that they can use the auxv version
if(REMUS_HAVE_AUXV_H)
  target_compile_definitions(RemusWorker PRIVATE REMUS_HAVE_AUXV_H)
endif()

#create the export header symbol exporting
remus_export_header(RemusWorker WorkerExports.h)

#install the library
remus_install_library(RemusWorker)

#install the headers
remus_public_headers(${headers})

#setup the exports for the library when used from a build tree
export(TARGETS RemusWorker
               RemusProto
               RemusCommon
               FILE RemusWorker-exports.cmake)

if(Remus_ENABLE_TESTING)
  add_subdirectory(testing)
endif()
