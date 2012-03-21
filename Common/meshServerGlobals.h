/*=========================================================================
  
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.
  
=========================================================================*/

#ifndef __MeshServerInfo_h
#define __MeshServerInfo_h

#include <string>
#include <sstream>

//Define global information that the mesh server needs

namespace meshserver {
static const int BROKER_PORT = 5555;

//these defines are used to specify services
static const std::string MESH2D="2DMESH";
static const std::string MESH3D="3DMESH";

inline std::string make_tcp_conn(const std::string& ip, int port)
{
  std::stringstream buffer;
  buffer << "tcp://" << ip << ":" <<port;
  return buffer.str();
}
}

#endif // __MeshServerInfo_
