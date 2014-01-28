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

#include <remus/client/JobContent.h>
#include <remus/testing/Testing.h>

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <ctime>


//we need a better name for JobContents, Data is a bad name I think that
//a better name could be JobContents, or JobInput

namespace {
using namespace remus::client;

std::string efficientStringGenerator(int length)
{
  std::string result;
  result.resize(length);

  std::string charset("abcdefghijklmnopqrstuvwxyz");
  std::random_shuffle(charset.begin(),charset.end());

  const std::size_t remainder = length % charset.length();
  const std::size_t times_to_copy = length / charset.length();

  //fill the remainder in first
  typedef std::string::iterator it;
  it start = result.begin();
  std::copy(charset.begin(), charset.begin()+remainder,start);

  std::random_shuffle(charset.begin(),charset.end());
  start += remainder;
  for(int i=0; i < times_to_copy; ++i, start+=charset.length())
    { std::copy(charset.begin(), charset.end(), start); }
  return result;
}

struct make_empty_string
{
  std::string operator()() const
  {
    return std::string();
  }

  std::size_t size() const
  {
    return 0;
  }

};

struct make_small_string
{
  std::string operator()() const
  {
    return std::string("small string");
  }
  std::size_t size() const
  {
    return std::string("small string").size();
  }
};

//makes rougly a 10mb sting
struct make_large_string
{
  std::string operator()() const
  {
    return efficientStringGenerator(this->size());
  }

  std::size_t size() const
  {
    return 10240*1024;
  }
};

//makes roughly a 2GB string
struct make_really_large_string
{
  std::string operator()() const
  {
    return efficientStringGenerator(this->size());
  }

  std::size_t size() const
  {
    std::string example;
    return (example.max_size() / 10);
  }
};


void verify_source_and_format()
{
  //validate that all the helpers work
  std::string path="example_data.txt";

  JobContent xml_file_test = make_FileJobContent(  ContentFormat::XML, path );
  JobContent json_file_test = make_FileJobContent( ContentFormat::JSON, path );
  JobContent bson_file_test = make_FileJobContent( ContentFormat::BSON, path );
  JobContent user_file_test = make_FileJobContent( ContentFormat::USER, path );

  REMUS_ASSERT( (xml_file_test.sourceType() == ContentSource::File ) );
  REMUS_ASSERT( (json_file_test.sourceType() == ContentSource::File ) );
  REMUS_ASSERT( (bson_file_test.sourceType() == ContentSource::File ) );
  REMUS_ASSERT( (user_file_test.sourceType() == ContentSource::File ) );

  REMUS_ASSERT( (xml_file_test.formatType() == ContentFormat::XML ) );
  REMUS_ASSERT( (json_file_test.formatType() == ContentFormat::JSON ) );
  REMUS_ASSERT( (bson_file_test.formatType() == ContentFormat::BSON ) );
  REMUS_ASSERT( (user_file_test.formatType() == ContentFormat::USER ) );

  JobContent xml_mem_test = make_MemoryJobContent( ContentFormat::XML, path );
  JobContent json_mem_test = make_MemoryJobContent( ContentFormat::JSON, path );
  JobContent bson_mem_test = make_MemoryJobContent( ContentFormat::BSON, path );
  JobContent user_mem_test = make_MemoryJobContent( ContentFormat::USER, path );

  REMUS_ASSERT( (xml_mem_test.sourceType() == ContentSource::Memory ) );
  REMUS_ASSERT( (json_mem_test.sourceType() == ContentSource::Memory ) );
  REMUS_ASSERT( (bson_mem_test.sourceType() == ContentSource::Memory ) );
  REMUS_ASSERT( (user_mem_test.sourceType() == ContentSource::Memory ) );

  REMUS_ASSERT( (xml_mem_test.formatType() == ContentFormat::XML ) );
  REMUS_ASSERT( (json_mem_test.formatType() == ContentFormat::JSON ) );
  REMUS_ASSERT( (bson_mem_test.formatType() == ContentFormat::BSON ) );
  REMUS_ASSERT( (user_mem_test.formatType() == ContentFormat::USER ) );
}

void verify_tag()
{
  std::string path="example_data.txt";
  JobContent test = make_FileJobContent(  ContentFormat::XML, path );
  test.tag("hello");
  REMUS_ASSERT( (test.tag() == "hello") );
}

template<typename StringFactory>
void verify_serilization_no_tag(StringFactory factory)
{
  JobContent input_content = make_MemoryJobContent(ContentFormat::XML, factory() );
  REMUS_ASSERT( (factory.size() == input_content.dataSize()) );

  REMUS_ASSERT( (input_content.tag().size() == 0 ) );
  REMUS_ASSERT( (input_content.tag() == std::string() ) );

  std::string wire_format = to_string(input_content);
  JobContent from_wire = to_JobContent(wire_format);

  REMUS_ASSERT( (from_wire.sourceType() == input_content.sourceType() ) );
  REMUS_ASSERT( (from_wire.formatType() == input_content.formatType() ) );
  REMUS_ASSERT( (from_wire.tag() == input_content.tag() ) );

  if(input_content.dataSize() > 0 )
    {
    REMUS_ASSERT( (from_wire.data() != NULL ) );
    REMUS_ASSERT( (input_content.data() != NULL ) );
    }
  else
    {
    REMUS_ASSERT( (from_wire.data() == NULL ) );
    REMUS_ASSERT( (input_content.data() == NULL ) );
    }
}

template<typename StringFactory>
void verify_serilization_with_tag(StringFactory factory)
{
  JobContent input_content = make_MemoryJobContent(ContentFormat::XML, factory() );
  REMUS_ASSERT( (factory.size() == input_content.dataSize()) );

  input_content.tag("we have a tag");
  REMUS_ASSERT( (input_content.tag().size() ==
                                  std::string("we have a tag").size() ) );
  REMUS_ASSERT( (input_content.tag() == "we have a tag" ) );

  std::string wire_format = to_string(input_content);
  JobContent from_wire = to_JobContent(wire_format);

  REMUS_ASSERT( (from_wire.sourceType() == input_content.sourceType() ) );
  REMUS_ASSERT( (from_wire.formatType() == input_content.formatType() ) );
  REMUS_ASSERT( (from_wire.tag() == input_content.tag() ) );

  REMUS_ASSERT( (input_content.tag() == "we have a tag" ) );
  REMUS_ASSERT( (from_wire.tag() == "we have a tag" ) );

  if(input_content.dataSize() > 0 )
    {
    REMUS_ASSERT( (from_wire.data() != NULL ) );
    REMUS_ASSERT( (input_content.data() != NULL ) );
    }
  else
    {
    REMUS_ASSERT( (from_wire.data() == NULL ) );
    REMUS_ASSERT( (input_content.data() == NULL ) );
    }
}

}

int UnitTestClientJobContent(int, char *[])
{
  //setup the random number generator
  std::srand(std::time(0));

  verify_source_and_format();
  verify_tag();

  std::cout << "make_empty_string" << std::endl;
  verify_serilization_no_tag( (make_empty_string()) );
  std::cout << "make_small_string" << std::endl;
  verify_serilization_no_tag( (make_small_string()) );
  std::cout << "make_large_string" << std::endl;
  verify_serilization_no_tag( (make_large_string()) );
  std::cout << "make_really_large_string" << std::endl;
  verify_serilization_no_tag( (make_really_large_string()) );

  std::cout << "make_empty_string" << std::endl;
  verify_serilization_with_tag( (make_empty_string()) );
  std::cout << "make_small_string" << std::endl;
  verify_serilization_with_tag( (make_small_string()) );
  std::cout << "make_large_string" << std::endl;
  verify_serilization_with_tag( (make_large_string()) );
  std::cout << "make_really_large_string" << std::endl;
  verify_serilization_with_tag( (make_really_large_string()) );
  return 0;
}