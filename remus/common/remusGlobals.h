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

#ifndef __remus_common_RemusInfo_h
#define __remus_common_RemusInfo_h

#include <string>

//Define global information that the mesh server needs

namespace remus {
static const int BROKER_CLIENT_PORT = 50505;
static const int BROKER_WORKER_PORT = 50510;
static const int HEARTBEAT_INTERVAL_IN_SEC = 5;
static const int HEARTBEAT_INTERVAL = 1000000 * HEARTBEAT_INTERVAL_IN_SEC;

static const std::string INVALID_MSG = "INVALID_MSG";

enum MESH_TYPE
{
  INVALID_MESH = 0,
  MESH2D = 2,
  MESH3D = 3,
  MESH3DSurface = 4
};

enum SERVICE_TYPE
{
  INVALID_SERVICE = 0,
  MAKE_MESH = 1,
  MESH_STATUS = 2,
  CAN_MESH = 3,
  RETRIEVE_MESH = 4,
  HEARTBEAT = 5,
  SHUTDOWN = 6
};

enum STATUS_TYPE
{
  INVALID_STATUS = 0,
  QUEUED = 1,
  IN_PROGRESS = 2,
  FINISHED = 3,
  FAILED = 4
};

namespace common
  {
  //a mapping of enum types to char*
  static const char *mesh_types[] = { "INVALID","", "2D", "3D", "3D Surface" };
  static const char *serv_types[] = { "INVALID", "MAKE MESH", "MESH STATUS", "CAN MESH", "RETRIEVE MESH", "HEARTBEAT", "SHUTDOWN" };
  static const char *stat_types[] = { "INVALID", "QUEUED", "IN PROGRESS", "FINISHED", "FAILED" };
  }

//------------------------------------------------------------------------------
inline std::string to_string(remus::MESH_TYPE t)
{
  return std::string(remus::common::mesh_types[(int)t]);
}

//------------------------------------------------------------------------------
inline std::string to_string(remus::SERVICE_TYPE t)
{
  return std::string(remus::common::serv_types[(int)t]);
}

//------------------------------------------------------------------------------
inline std::string to_string(remus::STATUS_TYPE t)
{
  return std::string(remus::common::stat_types[(int)t]);
}


//------------------------------------------------------------------------------
inline remus::MESH_TYPE to_meshType(const std::string& t)
{
  for(int i=2; i <=4; i++)
    {
    remus::MESH_TYPE mt=static_cast<remus::MESH_TYPE>(i);
    if (remus::to_string(mt) == t)
      {
      return mt;
      }
    }
  return remus::INVALID_MESH;
}

//------------------------------------------------------------------------------
inline remus::SERVICE_TYPE to_serviceType(const std::string& t)
{
  for(int i=1; i <=6; i++)
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

#endif // __remus_common_RemusInfo_h
