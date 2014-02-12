project(Remus_Proto)

set(headers
    Job.h
    JobContent.h
    JobProgress.h
    JobRequirements.h
    JobResult.h
    JobStatus.h
    JobSubmission.h
    zmq.hpp
    zmqHelper.h
    zmqSocketInfo.h
    zmqTraits.h
    )

set(srcs
    JobContent.cxx
    JobRequirements.cxx
    JobSubmission.cxx
    )

#setup the protocol library
add_library(RemusProto ${srcs} ${headers})

target_link_libraries(RemusProto
                      LINK_PUBLIC RemusCommon ${ZeroMQ_LIBRARIES}
                      )

target_include_directories(RemusProto
                           PUBLIC  ${ZeroMQ_INCLUDE_DIRS}
                                   ${Boost_INCLUDE_DIRS})

#create the export header symbol exporting
remus_export_header(RemusProto ProtoExports.h)

#install the protocol library
remus_install_library(RemusProto)

#install the headers
remus_public_headers(${headers})

#setup the exports for the library when used from a build tree
export(PACKAGE RemusProto)
export(TARGETS RemusProto
               RemusCommon
               sysTools
               FILE RemusProto-exports.cmake)


if(Remus_ENABLE_TESTING)
  add_subdirectory(testing)
endif()
