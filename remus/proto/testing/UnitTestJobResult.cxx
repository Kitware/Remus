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
#include <remus/proto/JobResult.h>
#include <remus/testing/Testing.h>

namespace {

using namespace remus::proto;

boost::uuids::uuid make_id()
{
  return remus::testing::UUIDGenerator();
}

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
  JobResult a(make_id());
  validate_serialization(a);

  JobResult b(make_id(),std::string());
  validate_serialization(b);

  JobResult c(make_id(),std::string("Contents"));
  validate_serialization(c);

  JobResult d(make_id(), remus::testing::BinaryDataGenerator(10240*10240) );
  validate_serialization(d);
}

}

int UnitTestJobResult(int, char *[])
{
  serialize_test();
  return 0;
}