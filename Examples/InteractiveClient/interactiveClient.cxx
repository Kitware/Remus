
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
std::map<remus::MESH_OUTPUT_TYPE,std::vector<remus::JobRequest> > AllJobTypeCombinations;
typedef std::map<remus::MESH_OUTPUT_TYPE,std::vector<remus::JobRequest> >::iterator AllTypeIt;
typedef std::vector<remus::JobRequest> RequestVector;
typedef std::vector<remus::JobRequest>::iterator RequestIt;

//populate the global memory mapping of job requests
void populateJobTypes()
{
  for(int i=1; i < remus::NUM_MESH_OUTPUT_TYPES; i++)
    {
    remus::MESH_OUTPUT_TYPE outType = static_cast<remus::MESH_OUTPUT_TYPE>(i);
    for(int j=1; j < remus::NUM_MESH_INPUT_TYPES; j++)
      {
      remus::MESH_INPUT_TYPE inType = static_cast<remus::MESH_INPUT_TYPE>(i);
      AllJobTypeCombinations[outType].push_back(remus::JobRequest(outType,inType));
      }
    }
}

void dumpMeshInputInfo(remus::Client &client, remus::MESH_OUTPUT_TYPE outType)
{
  RequestVector& requests = AllJobTypeCombinations[outType];
  for(RequestIt i=requests.begin(); i != requests.end(); ++i)
    {
    std::cout << "\t " << client.canMesh(*i) << std::endl;
    }
  std::cout << std::endl;
  return;
}

void dumpCanMeshInfo(remus::Client& client)
{
  std::cout << "Available Worker Types : " << std::endl;
  std::cout << "2DMesh: " << std::endl;
  dumpMeshInputInfo(client,remus::MESH2D);

  std::cout << "2DMesh: " << std::endl;
  dumpMeshInputInfo(client,remus::MESH3D);

  std::cout << "3D Surface: " << std::endl;
  dumpMeshInputInfo(client,remus::MESH3DSurface);
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
      remus::Job job(rawId,j->type());
      remus::JobStatus status = client.jobStatus(job);
      std::cout << " status of job is: " << status.Status << " " << remus::to_string(status.Status)  << std::endl;
      if(status.Status == remus::IN_PROGRESS)
        {
        std::cout << " progress is " << status.Progress << std::endl;
        }
      }
    }
}

void submitJob(remus::Client& client)
{
  for(int i=1; i < remus::NUM_MESH_INPUT_TYPES; i++)
    {
    remus::MESH_INPUT_TYPE mt=static_cast<remus::MESH_INPUT_TYPE>(i);
    std::cout << i << " " << remus::to_string(mt) << std::endl;
    }
  std::cout<<"Enter Job Input Type (integer): " << std::endl;
  int inType;
  std::cin >> inType;

  for(int i=1; i < remus::NUM_MESH_OUTPUT_TYPES; i++)
    {
    remus::MESH_OUTPUT_TYPE mt=static_cast<remus::MESH_OUTPUT_TYPE>(i);
    std::cout << i << " " << remus::to_string(mt) << std::endl;
    }
  std::cout<<"Enter Job Output Type (integer): " << std::endl;
  int outType;
  std::cin >> outType;

  std::string data;
  std::cout<<"Enter Job Data (string) : " << std::endl;
  std::cin >> data;

  remus::JobRequest request(static_cast<remus::MESH_OUTPUT_TYPE>(outType),
                            static_cast<remus::MESH_INPUT_TYPE>(inType),
                            data);
  remus::Job job = client.submitJob(request);

  std::cout << "Job Submitted, info is: " << std::endl;
  std::cout << "Job Valid: " << job.valid() << std::endl;
  std::cout << "Job Id: " << job.id() << std::endl;
  std::cout << "Job Input Type: " << remus::to_string(job.type().inputType()) << std::endl;
  std::cout << "Job Output Type: " << remus::to_string(job.type().outputType()) << std::endl;
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
