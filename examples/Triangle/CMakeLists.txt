
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
find_package(Triangle REQUIRED)

add_subdirectory(Client)
add_subdirectory(Worker)
