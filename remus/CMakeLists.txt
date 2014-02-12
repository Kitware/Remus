
add_subdirectory(testing)

#build the base common library
add_subdirectory(common)

#build the base common protocol library
#that all three components use to talk to each other
add_subdirectory(proto)


add_subdirectory(client)
add_subdirectory(server)
add_subdirectory(worker)

set(headers
  version.h
  )

remus_public_headers(${headers})

#setup the exports for the library when used from an installed location
install(EXPORT Remus-targets DESTINATION lib)
