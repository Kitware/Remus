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

#include <remus/client/JobContents.h>
#include <remus/testing/Testing.h>

//we need a better name for JobContents, Data is a bad name I think that
//a better name could be JobContents, or JobInput

int UnitTestJobContents(int, char *[])
{
  //validate that all the helpers work
  remus::client::JobContents file_test = remus::client::make_FileJobContents( path );
  remus::client::JobContents xml_test = remus::client::make_XMLJobContents( content );
  remus::client::JobContents json_test = remus::client::make_JSONJobContents( content );
  remus::client::JobContents bson_test = remus::client::make_BSONJobContents( content );
  remus::client::JobContents user_test = remus::client::make_UserJobContents( content );


  REMUS_ASSERT( (file_test.type() == remus::client::JobContents::File ) );
  REMUS_ASSERT( (xml_test.type() == remus::client::JobContents::Memory ) );
  REMUS_ASSERT( (json_test.type() == remus::client::JobContents::Memory ) );
  REMUS_ASSERT( (bson_test.type() == remus::client::JobContents::Memory ) );
  REMUS_ASSERT( (user_test.type() == remus::client::JobContents::Memory ) );

  REMUS_ASSERT( (xml_test.format_type() == remus::client::JobContents::XML() ) );
  REMUS_ASSERT( (json_test.format_type() == remus::client::JobContents::JSON() ) );
  REMUS_ASSERT( (bson_test.format_type() == remus::client::JobContents::BSON() ) );
  REMUS_ASSERT( (user_test.format_type() == remus::client::JobContents::USER() ) );

}