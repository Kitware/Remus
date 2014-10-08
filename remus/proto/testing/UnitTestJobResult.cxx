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

void validate_serialization(JobResult s,
remus::common::ContentFormat::Type ftype = remus::common::ContentFormat::User)
{
  std::string temp = to_string(s);
  JobResult from_string = to_JobResult(temp);

  REMUS_ASSERT( (from_string.id() == s.id()) );
  REMUS_ASSERT( (from_string.dataSize() == s.dataSize()) );
  REMUS_ASSERT( (from_string.valid() == s.valid()) );
  REMUS_ASSERT( (from_string.formatType() == ftype) );

  std::string data_from_string(from_string.data(),from_string.dataSize());
  std::string data_s(s.data(),s.dataSize());
  REMUS_ASSERT( (data_from_string == data_s) );


  std::stringstream buffer;
  JobResult from_buffer(make_id());
  buffer << s;
  buffer >> from_buffer;

  REMUS_ASSERT( (from_buffer.id() == s.id()) );
  REMUS_ASSERT( (from_buffer.dataSize() == s.dataSize()) );
  REMUS_ASSERT( (from_buffer.valid() == s.valid()) );
  REMUS_ASSERT( (from_buffer.formatType() == ftype) );

  std::string data_from_buffer(from_buffer.data(),from_buffer.dataSize());
  REMUS_ASSERT( (data_from_string == data_s) );

}

void serialize_test()
{
  JobResult a(make_id());
  validate_serialization(a);

  //test the make_JobResult with a string
  validate_serialization( make_JobResult( make_id(), std::string() ));

  validate_serialization( make_JobResult( make_id(),
                          remus::testing::UniqueString(),
                          remus::common::ContentFormat::JSON ),
                          remus::common::ContentFormat::JSON);

  validate_serialization( make_JobResult( make_id(),
                          remus::testing::AsciiStringGenerator(1024),
                          remus::common::ContentFormat::XML ),
                          remus::common::ContentFormat::XML);

  validate_serialization( make_JobResult( make_id(),
                          remus::testing::BinaryDataGenerator(10240),
                          remus::common::ContentFormat::BSON ),
                          remus::common::ContentFormat::BSON);

  //validate the constructor manually
  JobResult b(make_id(), remus::common::ContentFormat::User, std::string());
  validate_serialization(b);

  //validate the make_JobResult with a file handle
  validate_serialization( make_JobResult( make_id(),
                          remus::common::FileHandle( std::string() ),
                          remus::common::ContentFormat::JSON ),
                          remus::common::ContentFormat::JSON);

  validate_serialization( make_JobResult( make_id(),
                          remus::common::FileHandle(
                            remus::testing::AsciiStringGenerator(1024) ),
                          remus::common::ContentFormat::XML ),
                          remus::common::ContentFormat::XML);

  //validate the constructor manually
  JobResult c(make_id(), remus::common::ContentFormat::User,
              remus::common::FileHandle(std::string()));
  validate_serialization(c);
}

}

int UnitTestJobResult(int, char *[])
{
  serialize_test();
  return 0;
}