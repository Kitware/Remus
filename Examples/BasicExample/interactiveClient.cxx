
/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include <remus/client/Client.h>

#include <vector>
#include <iostream>


void dumpCanMeshInfo(remus::Client& client)
{
  std::cout << "Aviable Worker Types : " << std::endl;
  std::cout << "2DMesh: " << client.canMesh(remus::MESH2D) << std::endl;
  std::cout << "3DMesh: " << client.canMesh(remus::MESH3D) << std::endl;
  std::cout << "3D Surface: " << client.canMesh(remus::MESH3DSurface) << std::endl;
  return;
}

void dumpJobInfo(remus::Client& client)
{
  boost::uuids::uuid rawId;

  std::cout<<std::endl;
  std::cout<<"Enter Job Id: " << std::endl;
  std::cin >> rawId;

  //we will shotgun this job id as all three type
  for(int i=2; i<5; ++i)
    {
    const remus::MESH_TYPE type = static_cast<remus::MESH_TYPE>(i);
    remus::Job job(rawId,type);
    remus::JobStatus status = client.jobStatus(job);
    std::cout << " status of job is: " << status.Status << " " << remus::to_string(status.Status)  << std::endl;
    if(status.Status == remus::IN_PROGRESS)
      {
      std::cout << " progress is " << status.Progress << std::endl;
      }
    }
}


int showMenu()
{
  std::cout << "Options Are:" << std::endl;
  std::cout << "1: Dump Aviable Worker Types" << std::endl;
  std::cout << "2: Dump Info On Job" << std::endl;
  std::cout << std::endl;
  std::cout << "All other will quit application." << std::endl;
  int returnValue;
  std::cin >> returnValue;
  return returnValue;
}

int main ()
{

  std::cout << "What is the port you are connecting too: " << std::endl;
  int portNumber;
  std::cin >> portNumber;


  remus::client::ServerConnection conn("127.0.0.1",portNumber);
  remus::Client c(conn);

  bool wantInfo=true;
  while(wantInfo)
    {
    switch(showMenu())
      {
      case 1: //dump can mesh
        dumpCanMeshInfo(c);
        break;
      case 2:
        dumpJobInfo(c);
        break;
      default:
        wantInfo=false;
        break;
      };
    }
}
