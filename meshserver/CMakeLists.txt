
add_subdirectory(internal)

add_subdirectory(Broker)

set(headers
  client.h
  worker.h
  )

set_source_files_properties(${headers}
  PROPERTIES HEADER_FILE_ONLY TRUE
  )