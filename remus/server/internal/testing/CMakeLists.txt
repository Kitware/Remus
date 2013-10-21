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
  UnitTestActiveJobs.cxx
  UnitTestJobQueue.cxx
  UnitTestUUIDHelper.cxx
  UnitTestWorkerPool.cxx
  )

remus_unit_tests( SOURCES ${unit_tests}
                  LIBRARIES RemusCommon)