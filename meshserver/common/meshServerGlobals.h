/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __meshserver_common_MeshServerInfo_h
#define __meshserver_common_MeshServerInfo_h

#include <string>

//Define global information that the mesh server needs

namespace meshserver {
static const int BROKER_CLIENT_PORT = 5555;
static const int BROKER_WORKER_PORT = 5556;
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
  static const char *mesh_types[] = { "INVALID", "2D", "3D", "3D Surface" };
  static const char *serv_types[] = { "INVALID", "MAKE MESH", "MESH STATUS", "CAN MESH", "RETRIEVE MESH", "HEARTBEAT", "SHUTDOWN" };
  static const char *stat_types[] = { "INVALID", "QUEUED", "IN PROGRESS", "FINISHED", "FAILED" };
  }

//------------------------------------------------------------------------------
inline std::string to_string(meshserver::MESH_TYPE t)
{
  return std::string(meshserver::common::mesh_types[(int)t]);
}

//------------------------------------------------------------------------------
inline std::string to_string(meshserver::SERVICE_TYPE t)
{
  return std::string(meshserver::common::serv_types[(int)t]);
}

//------------------------------------------------------------------------------
inline std::string to_string(meshserver::STATUS_TYPE t)
{
  return std::string(meshserver::common::stat_types[(int)t]);
}


}

#endif // __meshserver_internal_MeshServerInfo_h
