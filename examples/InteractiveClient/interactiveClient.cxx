
/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include <remus/client/Client.h>

#include <vector>
#include <iostream>
#include <map>

//store a mapping of job output types to every jobrequest of that type.
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
  std::set< boost::shared_ptr<remus::meshtypes::MeshTypeBase> > allTypes =
    remus::common::MeshRegistrar::allRegisteredTypes();
  std::set< boost::shared_ptr<remus::meshtypes::MeshTypeBase> >::iterator iit = allTypes.begin();
  std::set< boost::shared_ptr<remus::meshtypes::MeshTypeBase> >::iterator jit = allTypes.begin();

  for(std::size_t i=1; i < numRegisteredMeshTypes; i++, ++iit)
    {
    MeshType outType = *iit;
    jit = allTypes.begin();
    for(std::size_t j=1; j < numRegisteredMeshTypes; j++, ++jit)
      {
      MeshType inType = *jit;
      const remus::common::MeshIOType mtype( (*inType),(*outType) );
      AllJobTypeCombinations[outType->name()].push_back( mtype );
      }
    }
}

void dumpMeshInputInfo(remus::Client &client,
                       const remus::meshtypes::MeshTypeBase& outType)
{
  bool atLeastOne = false;
  RequestVector requests = AllJobTypeCombinations[outType.name()];
  for(RequestIt i=requests.begin(); i != requests.end(); ++i)
    {
    if (client.canMesh(*i))
      {
      std::cout
        << "\t from "
        << remus::common::MeshRegistrar::instantiate((*i).inputType())->name()
        << " (" << (*i).inputType() << ")" << std::endl;
      atLeastOne = true;
      }
    }

  if (!atLeastOne)
    std::cout << "\t cannot be generated." << std::endl;

  std::cout << std::endl;
  return;
}

void dumpCanMeshInfo(remus::Client& client)
{
  std::cout << "Available Worker Types : " << std::endl;
  std::set< boost::shared_ptr<remus::meshtypes::MeshTypeBase> > allTypes =
    remus::common::MeshRegistrar::allRegisteredTypes();
  std::set< boost::shared_ptr<remus::meshtypes::MeshTypeBase> >::iterator iit;
  for (iit = allTypes.begin(); iit != allTypes.end(); ++iit)
    {
    std::cout << (*iit)->name() << ": " << std::endl;
    dumpMeshInputInfo(client, *iit->get());
    }

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
      std::cout << " status of job is: " << std::endl;
      std::cout << remus::proto::to_string(status);
      }
    }
}

void submitJob(remus::Client& client)
{
  std::cout << "Available Job I/O Types: " << std::endl;
  std::set< boost::shared_ptr<remus::meshtypes::MeshTypeBase> > allTypes =
    remus::common::MeshRegistrar::allRegisteredTypes();
  std::set< boost::shared_ptr<remus::meshtypes::MeshTypeBase> >::iterator iit;
  for (iit = allTypes.begin(); iit != allTypes.end(); ++iit)
    std::cout << (*iit)->name() << ": " << std::endl;

  std::cout<<"Enter Job Input Type (string): " << std::endl;
  std::string inType;
  std::cin >> inType;

  std::cout<<"Enter Job Output Type (string): " << std::endl;
  std::string outType;
  std::cin >> outType;

  std::string data;
  std::cout<<"Enter Job Data (string) : " << std::endl;
  std::cin >> data;

  MeshType in = remus::meshtypes::to_meshType(inType);
  MeshType out = remus::meshtypes::to_meshType(outType);

  remus::common::MeshIOType mtypes( *(in.get()),
                                     *(out.get()) );

  std::cout<<"Enter Job Worker Name (string): " << std::endl;
  remus::proto::JobRequirementsSet reqSet;
  remus::proto::JobRequirementsSet::const_iterator wit;
  if (client.canMesh(mtypes))
    {
    reqSet = client.retrieveRequirements(mtypes);

    if (reqSet.size())
      std::cout << "  Worker can be one of:" << std::endl;
    else
      std::cout << "  No workers found even though canMesh!" << std::endl;
    for (wit = reqSet.begin(); wit != reqSet.end(); ++wit)
      {
      std::cout << "    " << wit->workerName() << "\n";
      }
    }
  std::string workerName;
  std::cin >> workerName;
  for (wit = reqSet.begin(); wit != reqSet.end(); ++wit)
    if (wit->workerName() == workerName)
      break;

  // Create requirements, possibly using requirements from the named worker.
  remus::proto::JobRequirements request =
    remus::proto::make_JobRequirements(mtypes, workerName,
      (wit != reqSet.end() && wit->hasRequirements()) ? wit->requirements() : "");

  remus::proto::JobSubmission jsub(request);
  if (!data.empty())
    jsub.insert(
      remus::proto::JobSubmission::value_type(
        jsub.default_key(),
        remus::proto::JobContent(
          remus::common::ContentFormat::User,
          data)));

  remus::proto::Job job = client.submitJob(jsub);

  std::cout << "Job Submitted, info is: " << std::endl;
  std::cout << "Job Valid: " << job.valid() << std::endl;
  std::cout << "Job Id: " << job.id() << std::endl;
  std::cout << "Job Input Type: " << job.type().inputType() << std::endl;
  std::cout << "Job Output Type: " << job.type().outputType() << std::endl;
  return;
}

void changeConnection(remus::Client*& c)
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
  std::cout << "1: Dump Available Worker Types" << std::endl;
  std::cout << "2: Dump Info On Job" << std::endl;
  std::cout << "3: Submit Job" << std::endl;
  std::cout << "9: Connect to different server" << std::endl;
  std::cout << std::endl;
  std::cout << "Any other non-whitespace character will quit application." << std::endl;
  int returnValue;
  std::cin >> returnValue;
  return returnValue;
}

int main ()
{
  //populate the global job requests mapping
  populateJobTypes();

  remus::Client *c = NULL;
  changeConnection(c);

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
        changeConnection(c);
      default:
        wantInfo=false;
        break;
      };
    }
}
