
#Use the CMake'd zermMQ for windows, it isn't ready for linux/osx yet
add_external_project(zeroMQ
  CMAKE_ARGS
    -DBUILD_SHARED_LIBS:BOOL=ON
    -DZMQ_BUILD_FRAMEWORK:BOOL=OFF
  )

ExternalProject_Get_Property(zeroMQ install_dir)
add_project_property(zeroMQ ZeroMQ_ROOT_DIR ${install_dir})
