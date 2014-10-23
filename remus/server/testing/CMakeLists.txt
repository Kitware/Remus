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

set(unit_tests
  UnitTestCustomWorkerFactory.cxx
  UnitTestServer.cxx
  UnitTestServerMonitoring.cxx
  UnitTestWorkerFactory.cxx
  UnitTestServerPorts.cxx
  )

#to properly test worker factory we need an executable to run.
#This way we can properly run the exectuable
set(testing_workers
    TestWorker.cxx
    )

include(${Remus_SOURCE_DIR}/CMake/RemusRegisterWorker.cmake)


remus_register_unit_test_worker(EXEC_NAME TestWorker
                                INPUT_TYPE  "Edges"
                                OUTPUT_TYPE "Mesh2D"
                                CONFIG_DIR  "${CMAKE_CURRENT_BINARY_DIR}"
                                FILE_EXT   "tst")

remus_register_unit_test_worker(EXEC_NAME TestWorker
                                INPUT_TYPE  "Evil"
                                OUTPUT_TYPE "Good"
                                CONFIG_DIR  "${CMAKE_CURRENT_BINARY_DIR}"
                                FILE_EXT   "aet"
                                ENVIRONMENT
                                  "REMUS_WORKER_ENV_TEST" "TRUE"
                                  "NAUGHTY" "FALSE"
                                ARGUMENTS   "-argtest" "@SELF@"
                                TAG         "{\"thing\":\"yes\"}"
                                )

remus_register_unit_test_worker(EXEC_NAME TestWorker
                                INPUT_TYPE  "Edges"
                                OUTPUT_TYPE "Mesh2D"
                                CONFIG_DIR  "${CMAKE_CURRENT_BINARY_DIR}"
                                FILE_EXT   "fbr"
                                IS_FILE_BASED)

#state this executable is required by unit_tests and should be placed
#in the same location as the unit tests
remus_unit_test_executable(EXEC_NAME TestWorker SOURCES ${testing_workers})
target_link_libraries(TestWorker LINK_PRIVATE RemusCommon )

#generate the factory paths header
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/UnitTestWorkerFactoryPaths.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/UnitTestWorkerFactoryPaths.h
               @ONLY)

#we need to explicitly link to zmq for server ports test, which
#calls zmq directly to verify that the binding logic in ServerPorts
#is correct
remus_unit_tests(SOURCES ${unit_tests}
                 LIBRARIES RemusServer RemusProto
                 ${ZeroMQ_LIBRARIES} ${Boost_LIBRARIES})
