
set(headers
	JobQueue.h
  MessageRouter.h
	)

remus_private_headers(${headers})

if(Remus_ENABLE_TESTING)
  add_subdirectory(testing)
endif()