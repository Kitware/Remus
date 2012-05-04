
#first we find the system package
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.45.0 COMPONENTS thread filesystem system date_time REQUIRED)


# add dummy target so that we can attach properties and dependencies work properly
add_dummy_external_project(boost)

add_project_property(boost BOOST_INCLUDEDIR "${Boost_INCLUDE_DIR}")
add_project_property(boost BOOST_LIBRARYDIR "${Boost_LIBRARY_DIRS}")
