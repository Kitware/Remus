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


static const std::string ClientTag = "MDPC01";
static const std::string WorkerTag = "MDPW01";

enum MESH_TYPE
{
  MESH2D = 2,
  MESH3D = 3
};

enum SERVICE_TYPE
{
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
}

#endif // __MeshServerInfo_
