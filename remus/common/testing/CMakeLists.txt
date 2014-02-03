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

#to properly test execute process we need an executable to run.
#This way we can properly poll,kill,etc on the executable
set(testing_execute_process_executable
    TestExecutable.cxx
    )

#state this executable is required by unit_tests and should be placed
#in the same location as the unit tests
remus_unit_test_executable(EXEC_NAME TestExecutable
                           SOURCES ${testing_execute_process_executable}
                          )

#to launch this test executable we need a header that is configured
#with the correct path
configure_file(PathToTestExecutable.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/PathToTestExecutable.h
               @ONLY)

set(unit_tests
  UnitTestConditionalStorage.cxx
  UnitTestExecuteProcess.cxx
  UnitTestJobContent.cxx
  UnitTestJobProgress.cxx
  UnitTestMD5Hash.cxx
  UnitTestMeshIOType.cxx
  UnitTestMeshRegistry.cxx
  UnitTestSignalCatcher.cxx
  )

remus_unit_tests(SOURCES ${unit_tests}
                 LIBRARIES sysTools RemusCommon)
