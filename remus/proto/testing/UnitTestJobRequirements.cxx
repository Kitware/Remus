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

  std::size_t numRegisteredMeshTypes =
                      remus::common::MeshRegistrar::numberOfRegisteredTypes();

  //we want to work in some invalid mesh types so we say the min is 0
  int in_type = randomInt(0,numRegisteredMeshTypes);
  int out_type = randomInt(0,numRegisteredMeshTypes);

  return remus::common::MeshIOType( remus::meshtypes::to_meshType(in_type),
                                     remus::meshtypes::to_meshType(out_type)
                                     );


}

template<typename T>
JobRequirements make_MeshReqs(ContentFormat::Type ftype,
                              remus::common::MeshIOType mtype,
                              std::string name,
                              std::string tag,
                              T reqs)
{
  JobRequirements jreqs(ftype,mtype,name,reqs);
  jreqs.tag(tag);
  return jreqs;
}

JobRequirements make_random_MeshReqs()
{
  //generate a randomly computed meshreqs, will at times not
  //fill in the following optional flags:
  // tag.
  ContentSource::Type type =
                        randomEnum(ContentSource::File,ContentSource::Memory);
  if(type == ContentSource::File)
    {
    return make_MeshReqs( randomEnum(ContentFormat::User,ContentFormat::BSON),
                           randomMeshTypes(),
                           randomString(),
                           randomString(),
                           remus::common::FileHandle(randomString()) );
    }
  else
    {
    return make_MeshReqs( randomEnum(ContentFormat::User,ContentFormat::BSON),
                           randomMeshTypes(),
                           randomString(),
                           randomString(),
                           randomBinaryData() );
    }
}



void verify_tag()
{
  const std::string tag_data("example tag data");
  remus::common::MeshIOType mtypes((remus::meshtypes::Model()),
                                   (remus::meshtypes::Model()) );

  JobRequirements reqs = make_MeshReqs( ContentFormat::User,
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

  JobRequirements reqs = make_MeshReqs( ContentFormat::User,
                                        mtypes,
                                        workerName,
                                        "example tag data",
                                        "");

  REMUS_ASSERT( (reqs.workerName() == workerName) );
  JobRequirements reqs2 = to_JobRequirements( to_string(reqs) );
  REMUS_ASSERT( (reqs.workerName() == reqs2.workerName()) );
}

void verify_reqs_length()
{
  const std::string workerName("fake_worker");

  remus::common::MeshIOType mtypes((remus::meshtypes::Model()),
                                   (remus::meshtypes::Model()) );

  {
  //test requirements length with raw string
  JobRequirements reqs = make_MeshReqs( ContentFormat::User,
                                        mtypes,
                                        workerName,
                                        "example tag data",
                                        "data that is 26 chars long");

  REMUS_ASSERT( (reqs.hasRequirements() == true) );
  REMUS_ASSERT( (reqs.requirementsSize() == 26) );
  JobRequirements reqs2 = to_JobRequirements( to_string(reqs) );
  REMUS_ASSERT( (reqs2.hasRequirements() == true) );
  REMUS_ASSERT( (reqs2.requirementsSize() == 26) );
  }


  {
  //test requirements length with FileHandle
  JobRequirements freqs = make_MeshReqs( ContentFormat::User,
                                        mtypes,
                                        workerName,
                                        "example tag data",
                                        (FileHandle("path/to/file")));

  REMUS_ASSERT( (freqs.hasRequirements() == true) );
  REMUS_ASSERT( (freqs.requirementsSize() == 12) );
  JobRequirements freqs2 = to_JobRequirements( to_string(freqs) );
  REMUS_ASSERT( (freqs2.hasRequirements() == true) );
  REMUS_ASSERT( (freqs2.requirementsSize() == 12) );
  }

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

  //the size of the reversed vector is equal to the sorted && uniqueified size
  //of the original vector, which can be less than 100.
  std::vector< JobRequirements > reqs_reversed(reqs.size());
  std::copy(reqs.begin(), reqs.end(), reqs_reversed.begin());

  //reverse the string and verify that it is reversed
  std::reverse(reqs_reversed.begin(),reqs_reversed.end());
  REMUS_ASSERT( std::equal(reqs.begin(), reqs.end(), reqs_reversed.rbegin()) );

  //resort the reversed ones and verify the < operator works
  std::sort(reqs_reversed.begin(),reqs_reversed.end());
  const bool rev_equal = std::equal(reqs.begin(),
                                    reqs.end(),
                                    reqs_reversed.begin());
  const bool rev_set_equal = std::equal(reqs_reversed.begin(),
                                        reqs_reversed.end(),
                                        set_reqs.begin());
  REMUS_ASSERT( ( rev_equal && rev_set_equal ) )
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
    REMUS_ASSERT( (reqs.meshTypes() == reqs_serialized.meshTypes()) );
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
    to_wire.insert( make_random_MeshReqs() );
    }

  std::stringstream buffer;
  buffer << to_wire;
  buffer >> from_wire;

  const bool same = std::equal(to_wire.begin(),
                               to_wire.end(),
                               from_wire.begin());

  REMUS_ASSERT( same );
}


}

int UnitTestJobRequirements(int, char *[])
{
  //setup the random number generator
  std::srand(static_cast<unsigned int>(std::time(0)));

  verify_tag();
  verify_worker_name();
  verify_reqs_length();

  verify_less_than_op();

  verify_serilization();

  verify_req_set();
  return 0;
}