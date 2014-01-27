//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//=============================================================================

#include <remus/common/JobProgress.h>
#include <remus/testing/Testing.h>


namespace
{
using namespace remus::common;
}

int UnitTestJobProgress(int, char *[])
{

 //try out all permutations of constructor
  JobProgress invalid;
  REMUS_ASSERT( (invalid.value() == -1) )
  REMUS_ASSERT( (invalid.message() == std::string()) )

  JobProgress progress_queued(remus::FINISHED);
  REMUS_ASSERT( (progress_queued == invalid) );

  JobProgress progress_finished(remus::FINISHED);
  REMUS_ASSERT( (progress_finished == invalid) );

  JobProgress progress_failed(remus::FAILED);
  REMUS_ASSERT( (progress_failed == invalid) );

  JobProgress progress_expired(remus::EXPIRED);
  REMUS_ASSERT( (progress_expired == invalid) );

  JobProgress progress_zero(remus::IN_PROGRESS);
  REMUS_ASSERT( (progress_zero != invalid) );

  JobProgress progress_one(0);
  REMUS_ASSERT( (progress_zero != invalid) );

  //explicit setting a progress value means it must be 1 to 100
  //while setting the status to IN_PROGRESS just mean set progress
  //value to 0
  REMUS_ASSERT( (progress_one != progress_zero) );

  JobProgress progress_negative(-1);
  REMUS_ASSERT( (progress_negative == progress_one) );

  JobProgress progress_200(200);
  REMUS_ASSERT( (progress_200.value() == 100) );

  JobProgress progress_valid(50);
  REMUS_ASSERT( (progress_valid.value() == 50) );

  JobProgress progress_valid2(100);
  REMUS_ASSERT( (progress_valid2.value() == progress_200.value()) );
  REMUS_ASSERT( (progress_valid2 == progress_200) );
  REMUS_ASSERT( (progress_valid2.message() == std::string() ) );

  JobProgress progress_msg("hello");
  REMUS_ASSERT( (progress_msg.value() == 0) );
  REMUS_ASSERT( (progress_msg.message() == "hello") );

  JobProgress progress_msg_negative(-1,progress_msg.message());
  REMUS_ASSERT( (progress_msg_negative != progress_msg) );
  REMUS_ASSERT( (progress_msg_negative.value() == 1) );
  REMUS_ASSERT( (progress_msg_negative.message() == progress_msg.message()) );

  JobProgress progress_msg_200(200,progress_msg.message());
  REMUS_ASSERT( (progress_msg_200 != progress_msg) );
  REMUS_ASSERT( (progress_msg_200.value() == 100) );
  REMUS_ASSERT( (progress_msg_200.message() == progress_msg.message()) );

  JobProgress progress_msg_valid(50,progress_msg.message());
  REMUS_ASSERT( (progress_msg_valid.value() == 50) );
  REMUS_ASSERT( (progress_msg_valid.message() == progress_msg.message()) );

  JobProgress progress_msg_valid2(100,progress_msg.message());
  REMUS_ASSERT( (  progress_msg_valid2 == progress_msg_200) );

  //copy constructor
  JobProgress aa(invalid);
  REMUS_ASSERT( (aa == invalid) );
  REMUS_ASSERT( !(aa != invalid) );

  //copy assignment
  JobProgress bb = progress_valid2;
  REMUS_ASSERT( (bb == progress_valid2) );
  REMUS_ASSERT( !(bb != progress_valid2) );

  return 0;
}
