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
  UnitTestWorker.cxx
  UnitTestWorkerServerConnection.cxx
  )

remus_unit_tests(SOURCES ${unit_tests}
                 LIBRARIES RemusWorker
                           ${Boost_LIBRARIES}
                           ${CMAKE_THREAD_LIBS_INIT})