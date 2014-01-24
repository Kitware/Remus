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

//we need a better name for JobContents, Data is a bad name I think that
//a better name could be JobContents, or JobInput

using namespace remus::client;

void verify_source_and_format()
{
  //validate that all the helpers work
  std::string path="example_data.txt";

  JobContent xml_file_test = make_FileJobContent(  ContentFormat::XML, path );
  JobContent json_file_test = make_FileJobContent( ContentFormat::JSON, path );
  JobContent bson_file_test = make_FileJobContent( ContentFormat::BSON, path );
  JobContent user_file_test = make_FileJobContent( ContentFormat::USER, path );

  REMUS_ASSERT( (xml_file_test.source_type() == ContentSource::File ) );
  REMUS_ASSERT( (json_file_test.source_type() == ContentSource::File ) );
  REMUS_ASSERT( (bson_file_test.source_type() == ContentSource::File ) );
  REMUS_ASSERT( (user_file_test.source_type() == ContentSource::File ) );

  REMUS_ASSERT( (xml_file_test.format_type() == ContentFormat::XML ) );
  REMUS_ASSERT( (json_file_test.format_type() == ContentFormat::JSON ) );
  REMUS_ASSERT( (bson_file_test.format_type() == ContentFormat::BSON ) );
  REMUS_ASSERT( (user_file_test.format_type() == ContentFormat::USER ) );


  JobContent xml_mem_test = make_MemoryJobContent( ContentFormat::XML, path );
  JobContent json_mem_test = make_MemoryJobContent( ContentFormat::JSON, path );
  JobContent bson_mem_test = make_MemoryJobContent( ContentFormat::BSON, path );
  JobContent user_mem_test = make_MemoryJobContent( ContentFormat::USER, path );

  REMUS_ASSERT( (xml_mem_test.source_type() == ContentSource::Memory ) );
  REMUS_ASSERT( (json_mem_test.source_type() == ContentSource::Memory ) );
  REMUS_ASSERT( (bson_mem_test.source_type() == ContentSource::Memory ) );
  REMUS_ASSERT( (user_mem_test.source_type() == ContentSource::Memory ) );

  REMUS_ASSERT( (xml_mem_test.format_type() == ContentFormat::XML ) );
  REMUS_ASSERT( (json_mem_test.format_type() == ContentFormat::JSON ) );
  REMUS_ASSERT( (bson_mem_test.format_type() == ContentFormat::BSON ) );
  REMUS_ASSERT( (user_mem_test.format_type() == ContentFormat::USER ) );
}

void verify_tag()
{
  std::string path="example_data.txt";
  JobContent test = make_FileJobContent(  ContentFormat::XML, path );
  test.tag("hello");
  REMUS_ASSERT( (test.tag() == "hello") );
}

void verify_serilization()
{
  std::string path="example_data.txt";
  JobContent input_content = make_MemoryJobContent(ContentFormat::XML, path);

  std::string wire_format = to_string(test);

  JobContent from_wire = to_JobContent(wire_format);

  REMUS_ASSERT( (from_wire.source_type() == input_content.source_type() ) );
  REMUS_ASSERT( (from_wire.format_type() == input_content.format_type() ) );
  REMUS_ASSERT( (from_wire.tag() == input_content.tag() ) );
}

int UnitTestJobContent(int, char *[])
{
  verify_source_and_format();
  verify_tag();
  verify_serilization();
  return 0;
}