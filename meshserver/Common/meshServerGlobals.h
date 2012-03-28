/*=========================================================================
  
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.
  
=========================================================================*/

#ifndef __MeshServerInfo_h
#define __MeshServerInfo_h

#include <string>

//Define global information that the mesh server needs

namespace meshserver {
static const int BROKER_CLIENT_PORT = 5555;
static const int BROKER_WORKER_PORT = 5556;
static const int BROKER_STATUS_PORT = 5557;

enum MESH_TYPE
{
  INVALID_MESH = 0,
  MESH2D = 2,
  MESH3D = 3
};

enum SERVICE_TYPE
{
  INVALID_SERVICE = 0,
  MAKE_MESH = 1,
  MESH_STATUS = 2,
  CAN_MESH = 3,
  RETRIEVE_MESH = 4
};

enum STATUS_TYPE
{
  INVALID = 0,
  QUEUED = 1,
  IN_PROGRESS = 2,
  FINISHED = 3,
  FAILED = 4
};

namespace internal
  {
  //a mapping of enum types to char*
  static const char *mesh_types[] = { "INVALID", "2D", "3D" };
  static const char *serv_types[] = { "INVALID", "MAKE MESH", "MESH STATUS", "SUPPORT MESH TYPE", "RETRIEVE MESH" };
  static const char *stat_types[] = { "INVALID", "QUEUED", "IN PROGRESS", "FINISHED", "FAILED" };
  }

//------------------------------------------------------------------------------
inline std::string to_string(meshserver::MESH_TYPE t)
{
  return std::string(meshserver::internal::mesh_types[(int)t]);
}

//------------------------------------------------------------------------------
inline std::string to_string(meshserver::SERVICE_TYPE t)
{
  return std::string(meshserver::internal::serv_types[(int)t]);
}

//------------------------------------------------------------------------------
inline std::string to_string(meshserver::STATUS_TYPE t)
{
  return std::string(meshserver::internal::stat_types[(int)t]);
}


}

#endif // __MeshServerInfo_
