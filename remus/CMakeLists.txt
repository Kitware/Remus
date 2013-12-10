
add_subdirectory(testing)

add_subdirectory(common)
add_subdirectory(client)
add_subdirectory(server)
add_subdirectory(worker)

set(headers
  version.h
  )

remus_public_headers(${headers})

#setup the exports for the library when used from an installed location
install(EXPORT Remus-targets DESTINATION lib)
