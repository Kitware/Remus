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

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/random_generator.hpp>
#include <remus/worker/JobResult.h>
#include <remus/testing/Testing.h>

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <ctime>

namespace {

using namespace remus::worker;
boost::uuids::random_generator generator;

void validate_serialization(JobResult s)
{
  std::string temp = to_string(s);
  JobResult from_string = to_JobResult(temp);

  REMUS_ASSERT( (from_string.JobId == s.JobId) );
  REMUS_ASSERT( (from_string.Data == s.Data) );
  REMUS_ASSERT( (from_string.valid() == s.valid()) );
}

void serialize_test()
{
  JobResult a(generator());
  validate_serialization(a);

  JobResult b(generator(),std::string());
  validate_serialization(b);

  JobResult c(generator(),std::string("Contents"));
  validate_serialization(c);

  JobResult d(generator(), remus::testing::BinaryDataGenerator(10240*10240) );
  validate_serialization(d);
}

}

int UnitTestWorkerJobResult(int, char *[])
{
  serialize_test();
  return 0;
}