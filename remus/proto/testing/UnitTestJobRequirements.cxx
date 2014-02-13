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

#include <remus/proto/JobRequirements.h>
#include <remus/common/MeshTypes.h>
#include <remus/testing/Testing.h>

#include <algorithm>
#include <set>
#include <vector>


namespace
{
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



void verify_tag()
{
  const std::string tag_data("example tag data");
  remus::common::MeshIOType mtypes((remus::meshtypes::Model()),
                                   (remus::meshtypes::Model()) );

  JobRequirements reqs = make_MeshReqs( ContentSource::Memory,
                                            ContentFormat::USER,
                                            mtypes,
                                            "worker_name",
                                            tag_data,
                                            "");
  REMUS_ASSERT( (reqs.tag() == tag_data) );

  //serialize and deserialize and verify it is valid
  JobRequirements reqs2 = to_JobRequirements( to_string(reqs) );
  REMUS_ASSERT( (reqs.tag() == reqs2.tag()) );
}

void verify_worker_name()
{
  const std::string workerName("worker_name");

  remus::common::MeshIOType mtypes((remus::meshtypes::Model()),
                                   (remus::meshtypes::Model()) );

  JobRequirements reqs = make_MeshReqs( ContentSource::Memory,
                                            ContentFormat::USER,
                                            mtypes,
                                            workerName,
                                            "example tag data",
                                            "");

  REMUS_ASSERT( (reqs.workerName() == workerName) );
  JobRequirements reqs2 = to_JobRequirements( to_string(reqs) );
  REMUS_ASSERT( (reqs.workerName() == reqs2.workerName()) );
}


void verify_less_than_op()
{
  //construct 1024 mesh requirements using random data, store them into
  //a vector and sort the vector. than walk the data to verify
  //everything is in sorted order

  std::vector< JobRequirements > reqs(100);
  std::generate(reqs.begin(),reqs.end(), make_random_MeshReqs );

  std::set< JobRequirements > set_reqs(reqs.begin(),reqs.end());

  std::sort(reqs.begin(),reqs.end());
  reqs.erase(std::unique(reqs.begin(),reqs.end()), reqs.end());

  //at this point the number in set and vector should be equal
  REMUS_ASSERT( (reqs.size() == set_reqs.size()) );
  REMUS_ASSERT( std::equal(reqs.begin(), reqs.end(), set_reqs.begin()) );

  std::vector< JobRequirements > reqs_reversed(100);
  std::copy(reqs.begin(), reqs.end(), reqs_reversed.begin());

  //reverse the string and verify that it is reversed
  std::reverse(reqs_reversed.begin(),reqs_reversed.end());
  REMUS_ASSERT( std::equal(reqs.begin(), reqs.end(), reqs_reversed.rbegin()) );

  //resort the reveresed ones and verify the < operator works
  std::sort(reqs_reversed.begin(),reqs_reversed.end());
  REMUS_ASSERT( std::equal(reqs.begin(), reqs.end(), reqs_reversed.begin()) );
}

void verify_serilization()
{
  for(int i=0; i < 2048; ++i)
  {
    //the game plan is to create a serialize a ton of mesh reqs in an
    //attempt to find a permutation that fails
    JobRequirements reqs = make_random_MeshReqs();
    JobRequirements reqs_serialized =
        to_JobRequirements( to_string(reqs) );

    REMUS_ASSERT( (reqs.sourceType() == reqs_serialized.sourceType()) );
    REMUS_ASSERT( (reqs.formatType() == reqs_serialized.formatType()) );
    REMUS_ASSERT( (reqs.jobMeshTypes() == reqs_serialized.jobMeshTypes()) );
    REMUS_ASSERT( (reqs.workerName() == reqs_serialized.workerName()) );
    REMUS_ASSERT( (reqs.tag() == reqs_serialized.tag()) );
    REMUS_ASSERT( (reqs.hasRequirements() == reqs_serialized.hasRequirements()) );
    REMUS_ASSERT( (reqs.requirementsSize() == reqs_serialized.requirementsSize()) );
  }

}


void verify_req_set()
{
  JobRequirementsSet to_wire, from_wire;
  for(int i=0; i < 1024; ++i)
    {
    to_wire.get().insert( make_random_MeshReqs() );
    }

  std::stringstream buffer;
  buffer << to_wire;
  buffer >> from_wire;

  const bool same = std::equal(to_wire.get().begin(),
                               to_wire.get().end(),
                               from_wire.get().begin());

  REMUS_ASSERT( same );
}


}

int UnitTestJobRequirements(int, char *[])
{
  //setup the random number generator
  std::srand(std::time(0));

  verify_tag();
  verify_worker_name();
  verify_less_than_op();

  verify_serilization();

  verify_req_set();
  return 0;
}