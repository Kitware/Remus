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

#include <remus/common/MD5Hash.h>

#include <remus/common/ConditionalStorage.h>

#include <RemusSysTools/MD5.h>

namespace
{
  std::string to_hash(const char* data, const std::size_t length)
  {
    RemusSysToolsMD5* hasher = RemusSysToolsMD5_New();
    RemusSysToolsMD5_Initialize(hasher);
    RemusSysToolsMD5_Append(hasher,
                       reinterpret_cast<const unsigned char*>(data),
                       static_cast<int>(length) );
    char hash[32];
    RemusSysToolsMD5_FinalizeHex(hasher, hash);
    RemusSysToolsMD5_Delete(hasher);
    return std::string(hash,32);
  }
}

namespace remus {
namespace common {

std::string MD5Hash(const remus::common::ConditionalStorage& storage)
{
  return to_hash(storage.data(),storage.size());
}

std::string MD5Hash(const std::string& data)
{
  return to_hash(data.data(),data.size());
}

std::string MD5Hash(const char* data, std::size_t length)
{
  return to_hash(data,length);
}

}
}
