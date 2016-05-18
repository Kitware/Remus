
/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include <remus/common/CompilerInformation.h>

#ifdef REMUS_MSVC
 #pragma warning(push)
 #pragma warning(disable:4996)
#endif

#include <remus/client/Client.h>
#include <remus/server/PortNumbers.h>
#include <remus/proto/zmqHelper.h>
#include <remus/proto/zmq.hpp>

#include <vector>
#include <iostream>
#include <map>

#include "cJSON.h"

void dumpSupportedIOTypes(remus::Client& client)
{
  std::cout << "Mesh IO Types that server supports : " << std::endl;
  remus::common::MeshIOTypeSet supportedTypes = client.supportedIOTypes();

  remus::common::MeshIOTypeSet::const_iterator i;
  for (i = supportedTypes.begin(); i != supportedTypes.end(); ++i)
    {
    std::cout << "Input Type: \"" << i->inputType() << "\", "
              << "Output Type: \"" << i->outputType() << "\"" << std::endl;
    }

  return;
}

void dumpCanMeshInfo(remus::Client& client)
{
  std::cout << "Worker Types ready to be launched : " << std::endl;

  remus::common::MeshIOTypeSet supportedTypes = client.supportedIOTypes();
  remus::common::MeshIOTypeSet::const_iterator i;
  for (i = supportedTypes.begin(); i != supportedTypes.end(); ++i)
    {
    if (client.canMesh(*i))
      {
      std::cout << "\t from " << i->inputType() << " to " << i->outputType() << std::endl;
      }
    }
}

void monitorServer(remus::Client& client, const std::string& hostname)
{
  //for now we are going to have to presume a default status port
  //binding
  remus::client::ServerConnection conn = client.connection();
  zmq::socket_t monitor(*conn.context(),ZMQ_SUB);

  //bind the socket
  zmq::socketInfo<zmq::proto::tcp> default_sub(hostname,
                                               remus::server::STATUS_PORT);
  zmq::connectToAddress(monitor,default_sub);

  //subscribe to everything
  zmq_setsockopt (monitor, ZMQ_SUBSCRIBE, NULL, 0);
  // Always listen for "stop"
  std::string stopSub = "stop";
  zmq_setsockopt (monitor, ZMQ_SUBSCRIBE, stopSub.c_str(), stopSub.size());

  std::cout << "Starting to monitor all status from server " << hostname << std::endl;
  zmq::message_t key;
  zmq::message_t data;
  while (true)
    {
    monitor.recv(&key);
    monitor.recv(&data);

    std::string keyStr( reinterpret_cast<char*>(key.data()), key.size() );

    if(keyStr == stopSub)
      {
      return;
      }

    cJSON *root = cJSON_Parse( reinterpret_cast<char*>(data.data()) );
    char *rendered = cJSON_Print(root);
    std::cout << "key: " << keyStr << std::endl;
    std::cout << "value:" << (rendered ? rendered : "(empty)") << std::endl;

    free(rendered);
    cJSON_Delete(root);
    }
}

void dumpJobInfo(remus::Client& client)
{
  boost::uuids::uuid rawId;

  std::cout<<std::endl;
  std::cout<<"Enter Job Id: " << std::endl;
  std::cin >> rawId;

  remus::common::MeshIOTypeSet supportedTypes = client.supportedIOTypes();

  //we will shotgun this job id with all supported Id types
  bool have_valid_job = false;
  remus::common::MeshIOTypeSet::const_iterator i;
  for(i = supportedTypes.begin(); i!= supportedTypes.end(); ++i)
    {
    remus::proto::Job job(rawId,*i);
    remus::proto::JobStatus status = client.jobStatus(job);
    if(status.valid())
      {
      std::cout << " status of job is: " << std::endl;
      std::cout << remus::proto::to_string(status);
      have_valid_job = true;
      }
    }

  if(!have_valid_job)
    {
    std::cout << "unable to find job info, most likely given a bad job id" << std::endl;
    }
}

void submitJob(remus::Client& client)
{
  std::cout << "Available Job I/O Types: " << std::endl;

  remus::common::MeshIOTypeSet serverSupportedTypes = client.supportedIOTypes();

  //now print out all the valid combinations
  remus::common::MeshIOTypeSet::const_iterator i;
  for (i = serverSupportedTypes.begin(); i != serverSupportedTypes.end(); ++i)
    {
    std::cout << "Input Type: \"" << i->inputType() << "\", "
              << "Output Type: \"" << i->outputType() << "\"" << std::endl;
    }

  std::cout<<"Enter Job Input Type (string): " << std::endl;
  std::string inType;
  std::cin >> inType;

  std::cout<<"Enter Job Output Type (string): " << std::endl;
  std::string outType;
  std::cin >> outType;

  std::string data;
  std::cout<<"Enter Job Data (string) : " << std::endl;
  std::cin >> data;

  remus::common::MeshIOType mtypes( inType, outType);

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

void changeConnection(remus::Client*& c, std::string& hostname)
{
  hostname="127.0.0.1";
  int portNumber=remus::server::CLIENT_PORT;

  std::cout << "Would you like to connect to the default server[y/n]:" << std::endl;
  std::string use_default="y";
  std::cin >> use_default;
  if(use_default[0] == 'n' || use_default[0] == 'n')
    {
    std::cout << "What is the ip to connect too [default=127.0.0.1]:" << std::endl;
    std::cin >> hostname;

    std::cout << "What is the port you are connecting to[default=50505]:" << std::endl;
    std::cin >> portNumber;
    }

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
  std::cout << "1: Get Servers Supported MeshIO Types" << std::endl;
  std::cout << "2: Get Servers MeshIO Types that have ready workers" << std::endl;
  std::cout << "3: Monitor Server Status" << std::endl;
  std::cout << "4: Get Info On a Job" << std::endl;
  std::cout << "5: Submit Job" << std::endl;
  std::cout << "9: Connect to different server" << std::endl;
  std::cout << std::endl;
  std::cout << "Any other non-whitespace character will quit application." << std::endl;
  int returnValue;
  std::cin >> returnValue;
  return returnValue;
}

int main ()
{
  std::string hostname;
  remus::Client *c = NULL;
  changeConnection(c, hostname);

  bool wantInfo=true;
  while(wantInfo)
    {
    switch(showMenu())
      {
      case 1: //dump supported IO types
        dumpSupportedIOTypes(*c);
        break;
      case 2: //dump can mesh
        dumpCanMeshInfo(*c);
        break;
      case 3:
        monitorServer(*c, hostname);
        break;
      case 4:
        dumpJobInfo(*c);
        break;
      case 5:
        submitJob(*c);
        break;
      case 9:
        changeConnection(c, hostname);
      default:
        wantInfo=false;
        break;
      };
    }
}

#ifdef REMUS_MSVC
 #pragma warning(pop)
#endif
