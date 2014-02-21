
/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include <remus/client/Client.h>

#include <vector>
#include <iostream>
#include <map>

//store a mappin of job output types to every jobrequest of that type.
//this way we can easily send out a query to the server for a single output type
typedef boost::shared_ptr<remus::meshtypes::MeshTypeBase> MeshType;
std::map<std::string,std::vector<remus::common::MeshIOType> > AllJobTypeCombinations;
typedef std::map<std::string,std::vector<remus::common::MeshIOType> >::iterator AllTypeIt;
typedef std::vector<remus::common::MeshIOType> RequestVector;
typedef std::vector<remus::common::MeshIOType>::iterator RequestIt;

//populate the global memory mapping of job requests
void populateJobTypes()
{
  std::size_t numRegisteredMeshTypes =
                      remus::common::MeshRegistrar::numberOfRegisteredTypes();

  for(int i=1; i < numRegisteredMeshTypes; i++)
    {
    MeshType outType = remus::meshtypes::to_meshType(i);
    for(int j=1; j < numRegisteredMeshTypes; j++)
      {
      MeshType inType = remus::meshtypes::to_meshType(j);
      AllJobTypeCombinations[outType->name()].push_back(
                 remus::common::MeshIOType(*(inType.get()),*(outType.get()) ));
      }
    }
}

void dumpMeshInputInfo(remus::Client &client,
                       const remus::meshtypes::MeshTypeBase& outType)
{
  RequestVector requests = AllJobTypeCombinations[outType.name()];
  for(RequestIt i=requests.begin(); i != requests.end(); ++i)
    {
    std::cout << "\t " << (*i).inputType() << ": "
              << client.canMesh(*i) << std::endl;
    }
  std::cout << std::endl;
  return;
}

void dumpCanMeshInfo(remus::Client& client)
{
  std::cout << "Available Worker Types : " << std::endl;
  std::cout << "2DMesh: " << std::endl;
  dumpMeshInputInfo(client, (remus::meshtypes::Mesh2D()) );

  std::cout << "3DMesh: " << std::endl;
  dumpMeshInputInfo(client, (remus::meshtypes::Mesh3D()) );

  std::cout << "3D Surface: " << std::endl;
  dumpMeshInputInfo(client, (remus::meshtypes::Mesh3DSurface()));
  return;
}


void dumpJobInfo(remus::Client& client)
{
  boost::uuids::uuid rawId;

  std::cout<<std::endl;
  std::cout<<"Enter Job Id: " << std::endl;
  std::cin >> rawId;

  //we will shotgun this job id as all input and output types
  for(AllTypeIt i = AllJobTypeCombinations.begin(); i!= AllJobTypeCombinations.end(); ++i)
    {
    for(RequestIt j = i->second.begin(); j != i->second.end(); ++j)
      {
      remus::proto::Job job(rawId,*j);
      remus::proto::JobStatus status = client.jobStatus(job);
      std::cout << " status of job is: " << status.status() << " " << remus::to_string(status.status())  << std::endl;
      if(status.inProgress())
        {
        std::cout << " progress is " << status.progress() << std::endl;
        }
      }
    }
}

void submitJob(remus::Client& client)
{
  std::size_t numRegisteredMeshTypes =
                      remus::common::MeshRegistrar::numberOfRegisteredTypes();
  for(int i=1; i < numRegisteredMeshTypes; i++)
    {
    std::cout << i << " ";
    std::cout << remus::meshtypes::to_meshType(i)->name()<< std::endl;
    }
  std::cout<<"Enter Job Input Type (integer): " << std::endl;
  int inType;
  std::cin >> inType;

  for(int i=1; i < numRegisteredMeshTypes; i++)
    {
    std::cout << i << " ";
    std::cout << remus::meshtypes::to_meshType(i)->name()<< std::endl;
    }
  std::cout<<"Enter Job Output Type (integer): " << std::endl;
  int outType;
  std::cin >> outType;

  std::string data;
  std::cout<<"Enter Job Data (string) : " << std::endl;
  std::cin >> data;

  MeshType in = remus::meshtypes::to_meshType(inType);
  MeshType out = remus::meshtypes::to_meshType(outType);

  remus::common::MeshIOType mtypes( *(in.get()),
                                     *(out.get()) );

  remus::proto::JobRequirements request =
    remus::proto::make_MemoryJobRequirements(mtypes,"",data);

  remus::proto::Job job = client.submitJob(request);

  std::cout << "Job Submitted, info is: " << std::endl;
  std::cout << "Job Valid: " << job.valid() << std::endl;
  std::cout << "Job Id: " << job.id() << std::endl;
  std::cout << "Job Input Type: " << job.type().inputType() << std::endl;
  std::cout << "Job Output Type: " << job.type().outputType() << std::endl;
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
  //populate the global job requests mapping
  populateJobTypes();

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
