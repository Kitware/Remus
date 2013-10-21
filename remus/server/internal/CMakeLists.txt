
set(headers
  ActiveJobs.h
	JobQueue.h
  WorkerPool.h
  uuidHelper.h
	)

remus_private_headers(${headers})

if(Remus_ENABLE_TESTING)
  add_subdirectory(testing)
endif()
