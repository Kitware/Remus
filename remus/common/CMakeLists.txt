project(Remus_Common)

set(headers
    ConditionalStorage.h
    ContentTypes.h
    ExecuteProcess.h
    FileHandle.h
    MD5Hash.h
    MeshIOType.h
    MeshRegistrar.h
    MeshTypes.h
    remusGlobals.h
    SignalCatcher.h
    )

#private common files that don't need
#be given to the users of remus, but 
#are needed by other remus libraries
set(private_headers
    PollingMonitor.h
    )

set(srcs
    ExecuteProcess.cxx
    MD5Hash.cxx
    MeshRegistrar.cxx
    PollingMonitor.cxx
    )

#setup the common library
add_library(RemusCommon ${srcs} ${headers})
target_link_libraries(RemusCommon
                      LINK_PRIVATE sysTools
                      )
target_include_directories(RemusCommon
                           PUBLIC  ${Boost_INCLUDE_DIRS}
                           PRIVATE ${sysTools_BINARY_DIR} )


#create the export header symbol exporting
remus_export_header(RemusCommon CommonExports.h)

#install the libraries
remus_install_library(RemusCommon)

#install the headers
remus_public_headers(${headers})
remus_private_headers(${private_headers})

#setup the exports for the library when used from a build tree
export(PACKAGE RemusCommon)
export(TARGETS RemusCommon sysTools FILE RemusCommon-exports.cmake)

if(Remus_ENABLE_TESTING)
  add_subdirectory(testing)
endif()
