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
  AlwaysAcceptServer.cxx
  QueryIOTypes.cxx
  ShareContext.cxx
  SimpleJobFlow.cxx
  TerminateQueuedJob.cxx
  )

remus_integration_tests(SOURCES ${unit_tests}
                        LIBRARIES ${Boost_LIBRARIES})


#also add other permutation for SubmitSmallJobs where
#we modify the number of workers and jobs
add_executable(IntegrationTests_SubmitJobs
               SubmitJobs.cxx)
target_link_libraries(IntegrationTests_SubmitJobs
                      LINK_PRIVATE
                        RemusClient
                        RemusWorker
                        RemusServer
                        ${Boost_LIBRARIES}
                      )

add_test(NAME SubmitSmallJobs1by1
         COMMAND IntegrationTests_SubmitJobs small)
add_test(NAME SubmitSmallJobs1by4
         COMMAND IntegrationTests_SubmitJobs small 1 4)
add_test(NAME SubmitSmallJobs2by4
         COMMAND IntegrationTests_SubmitJobs small 2 4)
add_test(NAME SubmitSmallJobs4by2
         COMMAND IntegrationTests_SubmitJobs small 4 2)
add_test(NAME SubmitSmallJobs4by120
         COMMAND IntegrationTests_SubmitJobs small 4 120)
add_test(NAME SubmitLargeJobs1by1
         COMMAND IntegrationTests_SubmitJobs large)
add_test(NAME SubmitLargeJobs1by4
         COMMAND IntegrationTests_SubmitJobs large 1 4)
add_test(NAME SubmitLargeJobs2by4
         COMMAND IntegrationTests_SubmitJobs large 2 4)
