
#first we find the system package
find_package(ZeroMQ REQUIRED)

# add dummy target so that we can attach properties and dependencies work properly
add_dummy_external_project(zeroMQ)


add_project_property(zeroMQ ZeroMQ_LIBRARIES ${ZeroMQ_LIBRARIES})
add_project_property(zeroMQ ZeroMQ_INCLUDE_DIR ${ZeroMQ_INCLUDE_DIR})
