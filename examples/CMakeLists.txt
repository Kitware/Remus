
add_subdirectory(BasicClient)
add_subdirectory(BasicWorker)
add_subdirectory(BasicServer)
add_subdirectory(InfiniteWorker)
add_subdirectory(InteractiveClient)
add_subdirectory(NoWorkerServer)

#===============================================================================
# Now optional build full workers for real mesh libraries
# We currently provide example triangle worker
option(Remus_ENABLE_Triangle_Woker "Enable Building Triangle Worker" ON)

if(Remus_ENABLE_Triangle_Woker)
  add_subdirectory(TriangleWorker)
endif()
