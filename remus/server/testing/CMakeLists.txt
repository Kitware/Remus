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


remus_get_includes(RemusServer ServerIncludes)

set(unit_tests
  UnitTestServer.cxx
  UnitTestWorkerFactory.cxx
  UnitTestServerPorts.cxx
  )

#to properly test worker factory we need an executable to run.
#This way we can properly run the exectuable
set(testing_workers
    TestWorker.cxx
    )

remus_register_unit_test_worker(EXEC_NAME TestWorker
                                INPUT_TYPE  "RAW EDGES"
                                OUTPUT_TYPE "2D"
                                CONFIG_DIR  "${CMAKE_CURRENT_BINARY_DIR}"
                                FILE_EXT   "tst")

#state this executable is required by unit_tests and should be placed
#in the same location as the unit tests
remus_unit_test_executable(EXEC_NAME TestWorker SOURCES ${testing_workers})

#generate the factory paths header
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/UnitTestWorkerFactoryPaths.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/UnitTestWorkerFactoryPaths.h
               @ONLY)

remus_unit_tests(SOURCES ${unit_tests}
                 LIBRARIES RemusServer)