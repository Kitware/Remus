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

#ifndef remus_common_StatusTypes_h
#define remus_common_StatusTypes_h

#include <string>

namespace remus {

static const std::string INVALID_MSG = "INVALID_MSG";

//------------------------------------------------------------------------------
// Status Type macros
#define StatusTypeMacros() \
     StatusTypeMacro(INVALID_STATUS, 0,"INVALID"), \
     StatusTypeMacro(QUEUED, 1, "QUEUED"), \
     StatusTypeMacro(IN_PROGRESS, 2, "IN PROGRESS"), \
     StatusTypeMacro(FINISHED, 3, "FINISHED"), \
     StatusTypeMacro(FAILED, 4, "FAILED"), \
     StatusTypeMacro(EXPIRED, 5, "EXPIRED")

//------------------------------------------------------------------------------
enum STATUS_TYPE
{
#define StatusTypeMacro(ID,NUM,NAME) ID = NUM
  StatusTypeMacros()
#undef StatusTypeMacro
};


//------------------------------------------------------------------------------
namespace common
  {
  //a mapping of enum types to char*
  static const char *stat_types[] = {
#define StatusTypeMacro(ID,NUM,NAME) NAME
    StatusTypeMacros()
#undef StatusTypeMacro
  };
  }

//------------------------------------------------------------------------------
inline std::string to_string(remus::STATUS_TYPE t)
{
  return std::string(remus::common::stat_types[(int)t]);
}

//------------------------------------------------------------------------------
inline remus::STATUS_TYPE to_statusType(const std::string& t)
{
  for(int i=1; i <=5; i++)
    {
    remus::STATUS_TYPE mt=static_cast<remus::STATUS_TYPE>(i);
    if (remus::to_string(mt) == t)
      {
      return mt;
      }
    }
  return remus::INVALID_STATUS;
}

}

#endif // remus_common_StatusTypes_h
