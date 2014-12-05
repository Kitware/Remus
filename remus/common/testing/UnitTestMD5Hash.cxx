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

#include <string>

#include <remus/common/MD5Hash.h>
#include <remus/common/ConditionalStorage.h>

#include <remus/testing/Testing.h>


int UnitTestMD5Hash(int, char *[])
{
  //construct an empty conditional storage
  remus::common::ConditionalStorage empty;
  std::string empty_storage_hash = remus::common::MD5Hash(empty);
  std::string empty_storage_hash_c = remus::common::MD5Hash(NULL,0);
  std::string empty_storage_hash_str = remus::common::MD5Hash( (std::string()) );

  REMUS_ASSERT( (empty_storage_hash == empty_storage_hash_c) );
  REMUS_ASSERT( (empty_storage_hash == empty_storage_hash_str) );

  std::string empty_string;
  remus::common::ConditionalStorage empty_str(empty_string);
  std::string empty_storage_hash2 = remus::common::MD5Hash(empty_str);
  REMUS_ASSERT( (empty_storage_hash == empty_storage_hash2) );

  std::string content("Copyright (c) Kitware, Inc.");
  remus::common::ConditionalStorage t;

  //verify that swapping a conditional storage constructs the same hash
  remus::common::ConditionalStorage temp(content);
  std::string temp_hash = remus::common::MD5Hash(temp);
  t.swap( temp );
  std::string t_hash = remus::common::MD5Hash(t);
  REMUS_ASSERT( (temp_hash == t_hash) );
  REMUS_ASSERT( (temp_hash != empty_storage_hash) );

  //now that t is empty rehash and verify
  REMUS_ASSERT( (remus::common::MD5Hash(temp) == empty_storage_hash) );

  //verify that t hash equals the hash of its contents
  REMUS_ASSERT( (remus::common::MD5Hash(content) == t_hash) );
  REMUS_ASSERT( (remus::common::MD5Hash(t.data(),t.size()) == t_hash) );

  //now lets hash some bigger chunks to verify that works.
  std::string binary_junk = remus::testing::BinaryDataGenerator(10240*1024);
  remus::common::ConditionalStorage bstorage(binary_junk);

  REMUS_ASSERT( (remus::common::MD5Hash(binary_junk) ==
                 remus::common::MD5Hash(bstorage)) );

  REMUS_ASSERT( (empty_storage_hash.size() == 32) );
  REMUS_ASSERT( (remus::common::MD5Hash(bstorage).size() == 32) );

  return 0;
}
