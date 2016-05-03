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

#include <remus/proto/JobSubmission.h>
#include <remus/testing/Testing.h>

namespace {
using namespace remus::common;
using namespace remus::proto;

int randomInt(int min_v, int max_v)
{
  const float random = ((float)std::rand()/(float)RAND_MAX);
  const std::size_t diff = (max_v - min_v);
  return static_cast<int>(min_v + (random * diff));
}


//this will generate a random string, with a 25% chance that the string
//is empty, which is on purpose to sniff out errors with empty strings.
//otherwise we generate a string that is anywhere from 1 character to
//10 million characters
std::string randomString()
{
  int doEmptyLength  = randomInt(0, 4);
  std::size_t length = 0;
  if(doEmptyLength>0)
    {
    length = randomInt(1, 20 * doEmptyLength);
    }
  return remus::testing::AsciiStringGenerator(length);
}

std::string randomBinaryData()
{
  int doEmptyLength  = randomInt(0, 4);
  std::size_t length = randomInt(0, 100000 * (10 * doEmptyLength));
  return remus::testing::BinaryDataGenerator(length);
}

template<typename T>
T randomEnum(T min_t, T max_t)
{
  return static_cast<T>(randomInt(min_t,max_t));
}

remus::common::MeshIOType randomMeshTypes()
{
  remus::common::MeshIOTypeSet allTypes = remus::common::generateAllIOTypes();

  int selected_index = randomInt(0, static_cast<int>(allTypes.size()) );
  int index=0;
  typedef remus::common::MeshIOTypeSet::const_iterator cit;
  for(cit i=allTypes.begin(); i != allTypes.end(); ++i, ++index)
    {
    if(index == selected_index)
      {
      return *i;
      }
    }
  return remus::common::MeshIOType();
}

template<typename T>
JobRequirements make_MeshReqs(ContentFormat::Type ftype,
                              remus::common::MeshIOType mtype,
                              std::string name,
                              std::string tag,
                              T reqs)
{
  JobRequirements r(ftype,mtype,name,reqs); r.tag(tag);
  return r;
}

JobRequirements make_random_MeshReqs()
{
  if(randomEnum(ContentSource::File,ContentSource::Memory) ==
                ContentSource::File)
    {
    return JobRequirements( randomEnum(ContentFormat::User,ContentFormat::BSON),
                            randomMeshTypes(),
                            randomString(),
                            remus::common::FileHandle(randomString()) );
    }
  else
    {
    return JobRequirements( randomEnum(ContentFormat::User,ContentFormat::BSON),
                        randomMeshTypes(),
                        randomString(),
                        randomBinaryData() );
    }
}

JobContent make_random_Content()
{
  if(randomEnum(ContentSource::File,ContentSource::Memory) ==
                ContentSource::File)
    {
    return JobContent( randomEnum(ContentFormat::User,ContentFormat::BSON),
                       remus::common::FileHandle(randomString()) );
    }
  else
    {
    return JobContent( randomEnum(ContentFormat::User,ContentFormat::BSON),
                       randomBinaryData() );
    }
}


std::pair<std::string,JobContent> make_random_MapPairs()
{
  if(randomEnum(ContentSource::File,ContentSource::Memory) ==
                 ContentSource::File)
    {
    return std::pair<std::string,JobContent>( randomString(),
        JobContent(randomEnum(ContentFormat::User,ContentFormat::BSON),
                   remus::common::FileHandle(randomString()) ) );
    }

  return std::pair<std::string,JobContent>( randomString(),
        JobContent(randomEnum(ContentFormat::User,ContentFormat::BSON),
                   randomBinaryData() ) );
}

void constructor_test()
{
  JobSubmission invalid_sub;
  invalid_sub = JobSubmission( make_random_MeshReqs() );
  JobSubmission randomReqs = JobSubmission( make_random_MeshReqs() );
  invalid_sub = randomReqs;

  REMUS_ASSERT( (invalid_sub == randomReqs) );


  JobSubmission single_content(make_random_MeshReqs(),make_random_Content());

  std::string defaultKey = single_content.default_key();


  REMUS_ASSERT( (single_content.size() == 1) );
  REMUS_ASSERT( (single_content.find(defaultKey) != single_content.end()) );
  REMUS_ASSERT( (single_content.find(defaultKey) == single_content.begin()) );

  JobContent default_content_old = single_content.find(defaultKey)->second;

  //try to override the default content, which should failed
  single_content.insert(
      std::pair<std::string,JobContent>(
        defaultKey,
        make_random_Content())
      );
  REMUS_ASSERT( (single_content.find(defaultKey)->second == default_content_old) );


  //verify that type and requirements haven't changed
  JobSubmission js = JobSubmission( make_random_MeshReqs() );
  JobSubmission copy_js(js);
  JobSubmission assign_js =js;

  REMUS_ASSERT( (js.type()==copy_js.type()) );
  REMUS_ASSERT( (js.type()==assign_js.type()) );

  REMUS_ASSERT( (js.requirements()==copy_js.requirements()) );
  REMUS_ASSERT( (js.requirements()==assign_js.requirements()) );

}

void insert_test()
{
  JobRequirements reqs = make_random_MeshReqs();
  std::map< std::string, JobContent > content;
  for(std::size_t i = 0;  i < size_t(1024); ++i)
    { content.insert(make_random_MapPairs()); }

  JobSubmission sub1(reqs,content);
  JobSubmission sub2(reqs);

  JobSubmission sub3(reqs);
  sub1.insert(sub1.begin(), sub1.end() );
  sub3.insert(sub1.begin(), sub1.end() );

  bool equal = std::equal(sub1.begin(), sub1.end(),
                          sub3.begin());
  REMUS_ASSERT( equal );

  equal = std::equal(sub1.begin(), sub1.end(),
                     content.begin());
  REMUS_ASSERT( equal );
}


void serialize_operator_test()
{ //first verify the << and >> operators work

  std::map< std::string, JobContent > content;
  for(std::size_t i = 0;  i < size_t(10); ++i)
    { content.insert(make_random_MapPairs()); }
  JobSubmission to_wire(make_random_MeshReqs(),content);

  JobSubmission from_wire;

  std::stringstream buffer;
  buffer << to_wire;
  buffer >> from_wire;

  {
  std::map< std::string, JobContent > different;
  std::set_difference(to_wire.begin(), to_wire.end(),
                      from_wire.begin(), from_wire.end(),
                      std::inserter(different,different.begin()));
  //should be no difference between the two, using set_difference to
  //test the sorting requirements of the map too
  REMUS_ASSERT( (different.size()==0) );
  }

  REMUS_ASSERT( (from_wire == to_wire) );

  //clear from_wire
  from_wire = JobSubmission(make_random_MeshReqs());
  REMUS_ASSERT( !(from_wire == to_wire) );

  //try one without content
  std::stringstream buffer2;
  to_wire = JobSubmission(make_random_MeshReqs());
  buffer2 << to_wire;
  buffer2 >> from_wire;

  REMUS_ASSERT( (from_wire == to_wire) );
}

void to_from_string_test()
{ //first verify the << and >> operators work

  std::map< std::string, JobContent > content;
  for(std::size_t i = 0;  i < size_t(10); ++i)
    { content.insert(make_random_MapPairs()); }
  JobSubmission to_wire(make_random_MeshReqs(),content);

  JobSubmission from_wire = to_JobSubmission(to_string(to_wire));
  {
  std::map< std::string, JobContent > different;
  std::set_difference(to_wire.begin(), to_wire.end(),
                      from_wire.begin(), from_wire.end(),
                      std::inserter(different,different.begin()));
  //should be no difference between the two, using set_difference to
  //test the sorting requirements of the map too
  REMUS_ASSERT( (different.size()==0) );
  }
  REMUS_ASSERT( (from_wire == to_wire) );


  //try one without content, and use the c string api

  to_wire = JobSubmission(make_random_MeshReqs());
  const std::string temp = to_string(to_wire);
  from_wire = to_JobSubmission(temp.c_str(),temp.size());

  REMUS_ASSERT( (from_wire == to_wire) );
}

void multiple_content_test()
{ //verify that a job submission with multiple key:values works properly

  for(int i=0; i < 32; ++i)
  { //first verify with random everything
  JobRequirements reqs = make_random_MeshReqs();
  std::map< std::string, JobContent > content;
  for(std::size_t j = 0;  j < size_t(64); ++j)
    { content.insert(make_random_MapPairs()); }

  JobSubmission sub(reqs,content);
  JobSubmission from_wire = to_JobSubmission(to_string(sub));
  REMUS_ASSERT( (sub==from_wire) );
  }

  for(int i=0; i < 512; ++i)
  { //test with asci/binary/asci order
  JobRequirements reqs = make_random_MeshReqs();
  JobSubmission sub(reqs);
  sub["a"]= remus::proto::make_JobContent(remus::testing::AsciiStringGenerator(128));
  sub["b"]= remus::proto::make_JobContent(remus::testing::BinaryDataGenerator(256));
  sub["c"]= remus::proto::make_JobContent(remus::testing::AsciiStringGenerator(128));
  REMUS_ASSERT( (sub.size()==3) );

  JobSubmission from_wire = to_JobSubmission(to_string(sub));
  REMUS_ASSERT( (from_wire.size()==3) );
  REMUS_ASSERT( (sub==from_wire) );

  }

}

} //namespace


int UnitTestJobSubmission(int, char *[])
{
  constructor_test();
  insert_test();
  serialize_operator_test();
  to_from_string_test();

  multiple_content_test();

  return 0;
}
