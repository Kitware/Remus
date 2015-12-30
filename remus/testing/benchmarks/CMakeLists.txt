#=============================================================================
#
#  Copyright (c) Kitware, Inc.
#  All rights reserved.
#  See LICENSE.txt for details.
#
#  This software is distributed WITHOUT ANY WARRANTY; without even
#  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
#  PURPOSE.  See the above copyright notice for more information.
#
#=============================================================================

#add in the Performance benchmarks as standalone executables that
#aren't part of the testing infrastructure for now

add_executable(ClientMessagePerformance ClientMessagePerformance.cxx)
add_executable(WorkerMessagePerformance WorkerMessagePerformance.cxx)
add_executable(ServerMessagePerformance ServerMessagePerformance.cxx)

target_link_libraries(ClientMessagePerformance
    LINK_PRIVATE RemusClient RemusWorker RemusServer ${Boost_LIBRARIES} )

target_link_libraries(ServerMessagePerformance
    LINK_PRIVATE RemusClient RemusWorker RemusServer ${Boost_LIBRARIES} )

target_link_libraries(WorkerMessagePerformance
    LINK_PRIVATE RemusClient RemusWorker RemusServer ${Boost_LIBRARIES} )
