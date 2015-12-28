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

#ifndef remus_common_ServiceTypes_h
#define remus_common_ServiceTypes_h

#include <string>

//Define global information that the mesh server needs
namespace remus {

//------------------------------------------------------------------------------
// Severice Type macros.
#define ServiceTypeMacros() \
     ServiceTypeMacro(INVALID_SERVICE, 0,"INVALID"), \
     ServiceTypeMacro(SUPPORTED_IO_TYPES, 1, "SUPPORTED IO TYPES"), \
     ServiceTypeMacro(MESH_REQUIREMENTS_FOR_IO_TYPE, 2, "MESH REQUIREMENTS IO TYPE"), \
     ServiceTypeMacro(CAN_MESH_REQUIREMENTS, 3, "CAN MESH REQUIREMENTS"), \
     ServiceTypeMacro(CAN_MESH_IO_TYPE, 4, "CAN MESH IO TYPE"), \
     ServiceTypeMacro(MAKE_MESH, 5, "MAKE MESH"), \
     ServiceTypeMacro(MESH_STATUS, 6, "MESH STATUS"), \
     ServiceTypeMacro(RETRIEVE_RESULT, 7, "RETRIEVE RESULT"), \
     ServiceTypeMacro(HEARTBEAT, 8, "HEARTBEAT"), \
     ServiceTypeMacro(TERMINATE_JOB, 9, "TERMINATE JOB"), \
     ServiceTypeMacro(TERMINATE_WORKER, 10, "TERMINATE WORKER")


//------------------------------------------------------------------------------
enum SERVICE_TYPE
{
#define ServiceTypeMacro(ID,NUM,NAME) ID = NUM
   ServiceTypeMacros()
#undef ServiceTypeMacro
};

//------------------------------------------------------------------------------
namespace common
  {
  //a mapping of enum types to char*
  static const char *serv_types[] = {
#define ServiceTypeMacro(ID,NUM,NAME) NAME
    ServiceTypeMacros()
#undef ServiceTypeMacro
  };
  }

//------------------------------------------------------------------------------
inline std::string to_string(remus::SERVICE_TYPE t)
{
  return std::string(remus::common::serv_types[(int)t]);
}

//------------------------------------------------------------------------------
inline remus::SERVICE_TYPE to_serviceType(const std::string& t)
{
  for(int i=1; i<=10; i++)
    {
    remus::SERVICE_TYPE mt=static_cast<remus::SERVICE_TYPE>(i);
    if (remus::to_string(mt) == t)
      {
      return mt;
      }
    }
  return remus::INVALID_SERVICE;
}

}

#endif // remus_common_ServiceTypes_h
