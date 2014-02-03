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

#include <remus/common/JobContent.h>
#include <remus/testing/Testing.h>

#include <algorithm>
#include <vector>
#include <set>

namespace {
using namespace remus::common;


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
    return remus::testing::BinaryDataGenerator(this->size());
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
    return remus::testing::BinaryDataGenerator(this->size());
  }

  std::size_t size() const
  {
    //make the size 1.0GB
    return (2 << 29);
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


void verify_container_algorithm_support()
{
  make_small_string str_factory;
  make_large_string large_str_factory;
  std::vector<JobContent> jc_vec(100);
  for(std::size_t i=0; i < 100; ++i)
  { jc_vec[i] = make_MemoryJobContent(ContentFormat::XML, str_factory() ); }

  std::set<JobContent> jc_set(jc_vec.begin(),jc_vec.end());
  REMUS_ASSERT( (jc_set.size() == 1) ) //all the items in the vector are the same

  jc_vec.erase(std::unique(jc_vec.begin(),jc_vec.end()), jc_vec.end());
  REMUS_ASSERT( (jc_vec.size() == 1) );

  for(std::size_t i=0; i < 100; ++i)
  { jc_vec[i] = make_MemoryJobContent(ContentFormat::XML, large_str_factory() ); }

  jc_set = std::set<JobContent>(jc_vec.begin(),jc_vec.end());

  std::sort(jc_vec.begin(),jc_vec.end());
  jc_vec.erase(std::unique(jc_vec.begin(),jc_vec.end()), jc_vec.end());
  REMUS_ASSERT( (jc_vec.size() == jc_set.size()) );
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
  wire_format = ""; //deallocate massive string

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

  REMUS_ASSERT( (from_wire == input_content) );
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
  wire_format = ""; //deallocate massive string

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

  REMUS_ASSERT( (from_wire == input_content) );
}

template<typename StringFactory>
void verify_zero_copy_serilization(StringFactory factory)
{
  std::string zeroCopyData = factory();
  JobContent input_content = JobContent(ContentFormat::XML,
                                        zeroCopyData.c_str(),
                                        zeroCopyData.size());

  REMUS_ASSERT( (factory.size() == input_content.dataSize()) );
  REMUS_ASSERT( (zeroCopyData.c_str() == input_content.data()) );
  bool same = std::equal(input_content.data(),
                         input_content.data()+input_content.dataSize(),
                         zeroCopyData.c_str());
  REMUS_ASSERT( same );

  std::string wire_format = to_string(input_content);
  JobContent from_wire = to_JobContent(wire_format);
  wire_format = ""; //deallocate massive string

  REMUS_ASSERT( (from_wire.dataSize() == input_content.dataSize()) );

  if(input_content.dataSize() > 0 )
    {
    REMUS_ASSERT( (from_wire.data() != NULL ) );
    REMUS_ASSERT( (input_content.data() != NULL ) );
    }
  else
    {
    REMUS_ASSERT( (from_wire.data() == NULL ) );
    //this is where we leak some implementation details. When you
    //are in zero copy mode and you pass in an pointer which is allocated
    //but whose size is zero you get back the valid pointer, which is
    //different compared to when we allocate and zero size gives back the
    //null pointer
    REMUS_ASSERT( (input_content.data() != NULL ) );
    }

  REMUS_ASSERT( (from_wire == input_content) );
}

}

int UnitTestJobContent(int, char *[])
{
  verify_source_and_format();
  verify_tag();

  verify_container_algorithm_support();

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

  std::cout << "make_empty_string" << std::endl;
  verify_zero_copy_serilization( (make_empty_string()) );
  std::cout << "make_small_string" << std::endl;
  verify_zero_copy_serilization( (make_small_string()) );
  std::cout << "make_large_string" << std::endl;
  verify_zero_copy_serilization( (make_large_string()) );
  std::cout << "make_really_large_string" << std::endl;
  verify_zero_copy_serilization( (make_really_large_string()) );
  return 0;
}