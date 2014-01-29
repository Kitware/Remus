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
#include <remus/client/JobResult.h>
#include <remus/testing/Testing.h>

namespace {

using namespace remus::client;
boost::uuids::random_generator generator;

void validate_serialization(JobResult s)
{
  std::string temp = to_string(s);
  JobResult from_string = to_JobResult(temp);

  REMUS_ASSERT( (from_string.id() == s.id()) );
  REMUS_ASSERT( (from_string.data() == s.data()) );
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

int UnitTestClientJobResult(int, char *[])
{
  serialize_test();
  return 0;
}