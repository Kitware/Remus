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
  UnitTestJob.cxx
  UnitTestJobContent.cxx
  UnitTestJobProgress.cxx
  UnitTestJobRequirements.cxx
  UnitTestJobResult.cxx
  UnitTestJobStatus.cxx
  UnitTestJobSubmission.cxx
  )

remus_unit_tests(SOURCES ${unit_tests}
                 LIBRARIES RemusProto)
