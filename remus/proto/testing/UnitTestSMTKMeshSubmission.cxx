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

#include <remus/proto/SMTKMeshSubmission.h>
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

void constructor_test()
{
  SMTKMeshSubmission invalid_sub;
  invalid_sub = SMTKMeshSubmission( make_random_MeshReqs() );
  SMTKMeshSubmission randomReqs = SMTKMeshSubmission( make_random_MeshReqs() );
  invalid_sub = randomReqs;

  REMUS_ASSERT( (invalid_sub.hasAllComponents() == false) );
  REMUS_ASSERT( (invalid_sub == randomReqs) );


  SMTKMeshSubmission fullcontent(make_random_MeshReqs());
  fullcontent.model(make_random_Content());
  fullcontent.attributes(make_random_Content());
  fullcontent.modelItemsToMesh(make_random_Content());

  REMUS_ASSERT( (fullcontent.hasAllComponents() == true ) );
  REMUS_ASSERT( (fullcontent.size() == 3 ) );

  REMUS_ASSERT( (fullcontent.find( fullcontent.model_key() ) != fullcontent.end()) );
  REMUS_ASSERT( (fullcontent.find( fullcontent.attribute_key() ) != fullcontent.end()) );
  REMUS_ASSERT( (fullcontent.find( fullcontent.modelItems_key() ) != fullcontent.end()) );


  JobContent model_old = fullcontent.find( fullcontent.model_key() )->second;

  //try to override the model content, which should fail
  fullcontent.insert(
      std::pair<std::string,JobContent>(
        fullcontent.model_key(),
        make_random_Content())
      );
  REMUS_ASSERT( (fullcontent.find(fullcontent.model_key())->second == model_old) );


  //verify that type and requirements haven't changed
  SMTKMeshSubmission ms = SMTKMeshSubmission( make_random_MeshReqs() );
  SMTKMeshSubmission copy_ms(ms);
  SMTKMeshSubmission assign_ms = ms;

  REMUS_ASSERT( (ms.type()==copy_ms.type()) );
  REMUS_ASSERT( (ms.type()==assign_ms.type()) );

  REMUS_ASSERT( (ms.requirements()==copy_ms.requirements()) );
  REMUS_ASSERT( (ms.requirements()==assign_ms.requirements()) );

}

void conversion_to()
{
  JobRequirements reqs = make_random_MeshReqs();
  JobSubmission sub(reqs);

  {
  SMTKMeshSubmission temp;
  sub[temp.model_key()] = make_random_Content();
  sub[temp.attribute_key()] = make_random_Content();
  sub[temp.modelItems_key()] = make_random_Content();
  }

  //now convert this to a wire format, and read it back as a SMTKMeshSubmission
  std::stringstream buffer;
  buffer << sub;


  SMTKMeshSubmission meshSub = to_SMTKMeshSubmission(buffer.str());
  REMUS_ASSERT( (meshSub.hasAllComponents() == true) );

  REMUS_ASSERT( (meshSub.model() == sub[meshSub.model_key()]) );
  REMUS_ASSERT( (meshSub.attributes() == sub[meshSub.attribute_key()]) );
  REMUS_ASSERT( (meshSub.modelItemsToMesh() == sub[meshSub.modelItems_key()]) );

  REMUS_ASSERT( (meshSub.type()==sub.type()) );
  REMUS_ASSERT( (meshSub.requirements()==sub.requirements()) );
}

void conversion_from()
{
  JobRequirements reqs = make_random_MeshReqs();
  SMTKMeshSubmission meshSub(reqs);
  meshSub.model(make_random_Content());
  meshSub.attributes(make_random_Content());
  meshSub.modelItemsToMesh(make_random_Content());

  //now convert this to a wire format, and read it back as a JobSubmission
  std::stringstream buffer;
  buffer << meshSub;

  JobSubmission sub = to_JobSubmission(buffer.str());

  REMUS_ASSERT( (meshSub.model() == sub[meshSub.model_key()]) );
  REMUS_ASSERT( (meshSub.attributes() == sub[meshSub.attribute_key()]) );
  REMUS_ASSERT( (meshSub.modelItemsToMesh() == sub[meshSub.modelItems_key()]) );

  REMUS_ASSERT( (meshSub.type()==sub.type()) );
  REMUS_ASSERT( (meshSub.requirements()==sub.requirements()) );
}

void string_api()
{
  JobRequirements reqs = make_random_MeshReqs();
  SMTKMeshSubmission meshSub(reqs);

  std::string modelContent = randomString();
  std::string attribContent = randomString();
  std::string itemsContent = randomString();

  meshSub.model( modelContent, ContentFormat::User );
  meshSub.attributes( attribContent, ContentFormat::User );
  meshSub.modelItemsToMesh( itemsContent, ContentFormat::User );

  //now convert this to a wire format, and read it back as a JobSubmission
  SMTKMeshSubmission meshSub2;
  std::stringstream buffer;
  buffer << meshSub;
  buffer >> meshSub2;

  REMUS_ASSERT( (meshSub2.hasAllComponents() == true) );

  REMUS_ASSERT( (meshSub.model() == meshSub2.model() ) );
  REMUS_ASSERT( (meshSub.attributes() == meshSub2.attributes() ) );
  REMUS_ASSERT( (meshSub.modelItemsToMesh() == meshSub2.modelItemsToMesh() ) );
}

void zero_copy_api()
{
  JobRequirements reqs = make_random_MeshReqs();
  SMTKMeshSubmission meshSub(reqs);

  std::string modelContent = randomString();
  std::string attribContent = randomString();
  std::string itemsContent = randomString();

  meshSub.model( modelContent.c_str(), modelContent.size(), ContentFormat::User );
  meshSub.attributes( attribContent.c_str(), attribContent.size(), ContentFormat::User );
  meshSub.modelItemsToMesh( itemsContent.c_str(), itemsContent.size(), ContentFormat::User );

  //now convert this to a wire format, and read it back as a JobSubmission
  SMTKMeshSubmission meshSub2;
  std::stringstream buffer;
  buffer << meshSub;
  buffer >> meshSub2;

  REMUS_ASSERT( (meshSub2.hasAllComponents() == true) );

  REMUS_ASSERT( (meshSub.model() == meshSub2.model() ) );
  REMUS_ASSERT( (meshSub.attributes() == meshSub2.attributes() ) );
  REMUS_ASSERT( (meshSub.modelItemsToMesh() == meshSub2.modelItemsToMesh() ) );
}


} //namespace


int UnitTestSMTKMeshSubmission(int, char *[])
{
  constructor_test();

  conversion_to();
  conversion_from();

  string_api();
  zero_copy_api();

  return 0;
}
