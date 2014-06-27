# Remus: A Short Users Guide. #

[![Build Status](https://travis-ci.org/robertmaynard/Remus.svg?branch=master)]
  (https://travis-ci.org/robertmaynard/Remus)

Remus goal is to make meshing 2 and 3 dimensional meshes easy.
This is done by insulating your program from the instability and licenses of the
current generation of meshers. It does this by providing the ability for
meshing to happen on remote machines using a basic client, server, worker design.

## Remus Layout ##

Remus is split into five distinct groups. These groups are Common, Proto,
Client, Server, and Worker.

Common is a collection of helper classes that usable by any of the other groups.
It contains useful abilities such as MD5 computation, signal catching, process
launching and monitoring, mesh type registration, and a conditional storage class.

Proto contains all the common classes that are used during serilization.
If an object is being sent from the client to the server, it will be in the
proto group.

Finally the last three are very self explanatory as they are the interfaces
to the client, server and worker components of Remus.

## Using the Client API ##

The client interface for Remus revolves around the ```remus::Client``` class,
and the communication objects in the ```remus::proto``` namespace.

The client uses a request reply strategy with the server, and each call
on the client will block while it waits for a response from the server.

### Server Connection ###
The first step is to construct a connection to the server. This is done by
constructing a ```ServerConnection```.

```cpp
//create a default server connection
remus::client::ServerConnection conn;

//create a client object that will connect to the default remus server
remus::Client client(conn);
```

You can also explicitly state the machine name and port
```cpp
remus::client::ServerConnection conn("meshing_host", 8080);
```

### Communication with Server ###

The communication pattern that remus uses between the client and
server is has a very simple flow. First the client ask
for the set of available workers that match a set of requirements, from that
it choses a single worker and crafts a job submission using that workers
requirements.

- Query for support of a given mesh input and output type
  ```bool canMesh( const remus::common::MeshIOType& meshtypes )```
- Query to see if an exact JobRequirement is supported
  ```bool canMesh( const remus::proto::JobRequirements& requirements )```
- Retrieve all JobRequirements that match a given mesh input and output type
  ```
  remus::proto::JobRequirementsSet retrieveRequirements(
                          const remus::common::MeshIOType& meshtypes );
  ```
- Submit a job
  ```remus::proto::Job submitJob( const remus::proto::JobSubmission& submission )```
- Monitor the job
  ```remus::proto::JobStatus jobStatus( const remus::proto::Job& job )```
- Retrieve the results
  ```remus::proto::JobResult retrieveResults( const remus::proto::Job& job )```

Lets put this all together and show to submit a job to the server
and retrieve the results.

```cpp
//ask for a worker with the given input and output types
remus::common::MeshIOType requestIOType( (remus::meshtypes::Edges()),
                                         (remus::meshtypes::Mesh2D()) );
bool canMesh = client.canMesh(requestIOType)
if(canMesh)
  {
  //ask the server for all the job requirements that match the given input
  //and output type
  remus::proto::JobRequirementsSet reqSet =
                                client.retrieveRequirements(requestIOType);

  //craft a job submission using the first job requirements
  remus::proto::JobSubmission sub( *(reqSet.begin()) );
  //attach key value data
  sub["data"] = remus::proto::make_MemoryJobContent( "Hello Worker" );

  remus::proto::Job j = client.submitJob(sub);
  remus::proto::JobStatus state = client.jobStatus(j);
  while(state.good())
    {
    state = client.jobStatus(j);
    }

  //finished returns true if the worker sent a result to the server
  //if the meshing crashed or failed finished returns false
  if(state.finished())
    {
    remus::proto::JobResult result = client.retrieveResults(j);
    }
  }
```

## Using the Worker API ##

The worker interface for remus revolves around the ```remus::Worker``` class.
The worker uses an asynchronous strategy where the worker pushes information to the
server and doesn't wait for a response. Between these messages the worker
is sending heartbeat messages to the server, these allow the server to monitor
workers and be able to inform client when workers crash. The worker supports
both blocking and non blocking ways of getting jobs. You should check the validity
of all jobs before processing them, as a client is able to terminate a job after
it is sent to you, but before you take it from the queue.

### Server Connection ###
The first step is to construct a connection to the server. This is done by
constructing a ```ServerConnection```.

```cpp
//create a default server connection
remus::worker::ServerConnection conn;

//create a worker object that will connect to the default remus server
//with a input model type and an output mesh3d type, and with an explicit
//name of ExampleWorker

remus::common::MeshIOType io_type =
                          remus::common::make_MeshIOType(Model(),Mesh3D());
remus::proto::JobRequirements requirements =
                          remus::proto::make_MemoryJobRequirements(io_type,
                                                            "ExampleWorker",
                                                            "");

remus::Worker worker(requirements,conn);
```

You can also explicitly state the machine name and port
```cpp
remus::worker::ServerConnection conn("meshing_host", 8080);
```

### Communication with Server ###

The communication flow between the worker and server is very simple.
The process can be broken into 3 major steps. Those steps are:

- Get a Job
  To get a single job in a blocking manner:
    ```remus::worker::Job getJob()```
  To get a single job in a non blocking manner:
    ```askForJobs(1)```
    ```remus::worker::Job takePendingJob()
- Send Job Status
  ```void updateStatus(const remus::proto::JobStatus& info)```
- Send Job Results
  ```void returnMeshResults(const remus::proto::JobResult& result)```

Lets put this all together and show how to get a job from the server,
send back some status, and than a result

```cpp

//keep asking for job till we have a valid job
remus::worker::Job j = worker.getJob();
while( j.valid() )
  { //a job could be invalid if a client terminates it, or if the server
    //crashes and has told us to shutdown (in that case we also need) to
    //check why the job isn't valid
  j = worker.getJob();
  }

//lets take the data key's value out of the job submission
const remus::proto::JobContent& content =
                                    jd.submission().find("data")->second;
std::string message(content.data(),content.dataSize());
message += " and Hello Client I am currently in progress";

remus::proto::JobProgress progress(message);
remus::proto::JobStatus status(j.id(),progress);

for(int i=0; i < 100; i+=10)
  {
  progress.setValue(i+1);
  worker.updateStatus( status.updateProgress(progress) );
  }

std::string finished_message(content.data(),content.dataSize());
finished_message += " and Hello Client I am now finished";
remus::proto::JobResult results(j.id(),finished_message);
worker.returnMeshResults(results);

```

## Calling an External Program ##

A common occurrence is a worker needs to call some external
command line program. Luckily Remus has built in support for executing processes.

```
using remus::common::ExecuteProcess;
using remus::common::ProcessPipe;

ExecuteProcess lsProcess("ls");
lsProcess.execute(ExecuteProcess::Attached);

ProcessPipe data = this->OmicronProcess->poll(-1);
if(data.type == ProcessPipe::STDOUT)
  {
  std::cout << data.text << std::endl;
  }
```

## Constructing a Remus Worker File ##

When you are creating custom workers that you want the default worker factory
to understand you need to generate a remus worker file. The files is a very
simple json file that looks like:

```
{
"ExecutableName": "ExampleWorker",
"InputType": "Model",
"OutputType": "Mesh3D"
}
```

If you have a worker that has explicit job requirements from a file you can
specify it like so:

```
{
"ExecutableName": "ExampleWorker",
"InputType": "Model",
"OutputType": "Mesh3D",
"File": "<path>",
#valid options are JSON, BSON, XML and USER
"FileFormat" : "JSON"
}
```

## Register a New Mesh Type ##

Remus can be extended to support custom defined mesh types, if the default
one are insufficient. Lets say that you have a meshing process whose input
is actually an AMR dataset and whose output is collection of Voxels. You can
do the following to have Remus support those new types.

Remus reserves the id space 0 to 100 for provided mesh types. User defined
mesh types should start at 100. If you are working with multiple plugins that
define mesh types, I would ask the registrar for all registered types and verify
the id spaces don't collide before adding new ids.

```cpp
  #include <remus/common/MeshTypes.h>

  //lets derive a new type from mesh registry
  struct AMRData : remus::meshtypes::MeshTypeBase
  {
  static boost::shared_ptr<MeshTypeBase> create() {
          return boost::shared_ptr<AMRData>(new AMRData()); }
  boost::uint16_t id() const { return 1234; }
  std::string name() const { return "AMRData"; }
  };

  //lets derive a new type from mesh registry
  struct Voxel : remus::meshtypes::MeshTypeBase
  {
  static boost::shared_ptr<MeshTypeBase> create() {
          return boost::shared_ptr<Voxel>(new Voxel()); }
  boost::uint16_t id() const { return 1235; }
  std::string name() const { return "Voxel"; }
  };

  remus::common::MeshRegistrar a( (AMRData()) );
  remus::common::MeshRegistrar b( (Voxel()) );
```

This will need to be compiled into a custom Server if you need the default
worker factory to parse these types from a Remus Worker json file. If you just
need it on the client and worker, you can just define the above in just the
client and worker implementations.

now you can use the AMRData and Voxel classes in the client and worker, like
this:

```
remus::common::MeshIOType io_type =
                          remus::common::make_MeshIOType(AMRData(),Voxel());
```
