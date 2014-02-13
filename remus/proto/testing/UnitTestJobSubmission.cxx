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
  return min_v + (random * diff);
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

  std::size_t numRegisteredMeshTypes =
                      remus::common::MeshRegistrar::numberOfRegisteredTypes();

  //we want to work in some invalid mesh types so we say the min is 0
  int in_type = randomInt(0,numRegisteredMeshTypes);
  int out_type = randomInt(0,numRegisteredMeshTypes);

  return remus::common::MeshIOType( remus::meshtypes::to_meshType(in_type),
                                     remus::meshtypes::to_meshType(out_type)
                                     );


}

JobRequirements make_MeshReqs(ContentSource::Type stype,
                                  ContentFormat::Type ftype,
                                  remus::common::MeshIOType mtype,
                                  std::string name,
                                  std::string tag,
                                  std::string reqs)
{
  return JobRequirements(stype,ftype,mtype,name,tag,reqs);
}

JobRequirements make_random_MeshReqs()
{
  //generate a randomly computed meshreqs, will at times not
  //fill in the following optional flags:
  // tag.
  return make_MeshReqs( randomEnum(ContentSource::File,ContentSource::Memory),
                        randomEnum(ContentFormat::USER,ContentFormat::BSON),
                        randomMeshTypes(),
                        randomString(),
                        randomString(),
                        randomBinaryData() );
}


std::pair<std::string,JobContent> make_random_MapPairs()
{
  return std::pair<std::string,JobContent>( randomString(),
        JobContent( randomEnum(ContentSource::File,ContentSource::Memory),
                      randomEnum(ContentFormat::USER,ContentFormat::BSON),
                      randomBinaryData() )
        );
}


}

void constructor_test()
{
  JobSubmission invalid_sub;
  invalid_sub = JobSubmission( make_random_MeshReqs() );
  JobSubmission randomReqs = JobSubmission( make_random_MeshReqs() );
  invalid_sub = randomReqs;

  REMUS_ASSERT( (invalid_sub == randomReqs) );


  JobSubmission single_content(make_random_MeshReqs(),
     JobContent( randomEnum(ContentSource::File,ContentSource::Memory),
                 randomEnum(ContentFormat::USER,ContentFormat::BSON),
                 randomBinaryData() ));

  REMUS_ASSERT( (single_content.size() == 1) );
  REMUS_ASSERT( (single_content.find("default") != single_content.end()) );
  REMUS_ASSERT( (single_content.find("default") == single_content.begin()) );

  JobContent default_content_old = single_content.find("default")->second;

  //try to override the default content, which should faild
  single_content.insert(
      std::pair<std::string,JobContent>(
      "default",
      JobContent( randomEnum(ContentSource::File,ContentSource::Memory),
                  randomEnum(ContentFormat::USER,ContentFormat::BSON),
                  randomBinaryData() )
        )
      );
  REMUS_ASSERT( (single_content.find("default")->second == default_content_old) );

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


void serialize_test()
{
  std::map< std::string, JobContent > content;
  for(std::size_t i = 0;  i < size_t(10); ++i)
    { content.insert(make_random_MapPairs()); }
  JobSubmission to_wire(make_random_MeshReqs(),content);

  JobSubmission from_wire;

  std::stringstream buffer;
  buffer << to_wire;
  buffer >> from_wire;

  std::cout << to_wire.size() << std::endl;
  std::cout << from_wire.size() << std::endl;

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


  //try one without content
  std::stringstream buffer2;
  to_wire = JobSubmission(make_random_MeshReqs());
  buffer2 << to_wire;
  buffer2 >> from_wire;

  std::cout << to_wire.size() << std::endl;
  std::cout << from_wire.size() << std::endl;


  REMUS_ASSERT( (from_wire == to_wire) );
}


int UnitTestJobSubmission(int, char *[])
{
  constructor_test();
  insert_test();
  serialize_test();

  return 0;
}