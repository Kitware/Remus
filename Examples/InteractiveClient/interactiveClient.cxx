
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
  std::cout << "Available Worker Types : " << std::endl;
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

void submitJob(remus::Client& client)
{
  for(int i=2; i <=4; i++)
    {
    remus::MESH_TYPE mt=static_cast<remus::MESH_TYPE>(i);
    std::cout << i << " " << remus::to_string(mt) << std::endl;
    }
  std::cout<<"Enter Job Type (integer): " << std::endl;
  int type;
  std::cin >>type;

  std::string data;
  std::cout<<"Enter Job Dat (string) : " << std::endl;
  std::cin >> data;

  remus::JobRequest request(static_cast<remus::MESH_TYPE>(type),data);
  remus::Job job = client.submitJob(request);

  std::cout << "Job Submitted, info is: " << std::endl;
  std::cout << "Job Valid: " << job.valid() << std::endl;
  std::cout << "Job Id: " << job.id() << std::endl;
  std::cout << "Job Type: " << remus::to_string(job.type()) << std::endl;
  return;
}

void changeConntion(remus::Client*& c)
{

  std::cout << "What is the ip to connect too: " << std::endl;
  std::string hostname;
  std::cin >> hostname;

  std::cout << "What is the port you are connecting to: " << std::endl;
  int portNumber;
  std::cin >> portNumber;


  remus::client::ServerConnection conn(hostname,portNumber);
  if(c != NULL)
    {
    delete c;
    }
  std::cout << conn.endpoint() << std::endl;
  c = new remus::Client(conn);

}


int showMenu()
{
  std::cout << "Options Are:" << std::endl;
  std::cout << "1: Dump Aviable Worker Types" << std::endl;
  std::cout << "2: Dump Info On Job" << std::endl;
  std::cout << "3: Submit Job" << std::endl;
  std::cout << "9: Connect to different server" << std::endl;
  std::cout << std::endl;
  std::cout << "All other will quit application." << std::endl;
  int returnValue;
  std::cin >> returnValue;
  return returnValue;
}

int main ()
{

  remus::Client *c = NULL;
  changeConntion(c);

  bool wantInfo=true;
  while(wantInfo)
    {
    switch(showMenu())
      {
      case 1: //dump can mesh
        dumpCanMeshInfo(*c);
        break;
      case 2:
        dumpJobInfo(*c);
        break;
      case 3:
        submitJob(*c);
        break;
      case 9:
        changeConntion(c);
      default:
        wantInfo=false;
        break;
      };
    }
}
