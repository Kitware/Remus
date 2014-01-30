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

#ifndef remus_common_MD5Hash_h
#define remus_common_MD5Hash_h

#include <string>
#include <remus/common/ConditionalStorage.h>

#include <remus/common/CommonExports.h>

namespace remus {
namespace common {

REMUSCOMMON_EXPORT
std::string MD5Hash(const remus::common::ConditionalStorage& storage);

REMUSCOMMON_EXPORT
std::string MD5Hash(const std::string& data);

REMUSCOMMON_EXPORT
std::string MD5Hash(const char* data, std::size_t length);

}
}

#endif