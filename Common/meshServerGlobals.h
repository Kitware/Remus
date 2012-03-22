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

enum MESH_TYPES
  {
  MESH2D = 1 << 1,
  MESH3D = 1 << 2
  };

}

#endif // __MeshServerInfo_
