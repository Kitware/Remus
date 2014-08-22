
add_subdirectory(BasicClient)
add_subdirectory(BasicServer)
add_subdirectory(BasicWorker)

add_subdirectory(InfiniteWorker)
add_subdirectory(InteractiveClient)
add_subdirectory(NoWorkerServer)
add_subdirectory(QtExample)
add_subdirectory(nuclear)

#===============================================================================
# Now optional build full workers for real mesh libraries
# We currently provide example triangle worker

option(Remus_ENABLE_Triangle_Worker "Enable Building Triangle Worker" OFF)
if(Remus_ENABLE_Triangle_Worker)
  add_subdirectory(Triangle)
endif()

option(Remus_ENABLE_TetGen_Worker "Enable Building TetGen Worker" OFF)
if(Remus_ENABLE_TetGen_Worker)
  add_subdirectory(TetGen)
endif()
