# Remus: A Short Users Guide. #

[![Imgur](http://i.imgur.com/8zsK3vl.png?2)](http://open.cdash.org/index.php?project=Remus)
[![Build Status](http://cbadges.com/Remus/build/robertmaynard/Remus/master)](http://open.cdash.org/index.php?project=Remus)
[![Test Status](http://cbadges.com/Remus/test/robertmaynard/Remus/master)](http://open.cdash.org/index.php?project=Remus)
[![Coverage Status](http://cbadges.com/Remus/coverage/robertmaynard/Remus/master)](http://open.cdash.org/index.php?project=Remus)

Remus' goal is to make meshing 2 and 3 dimensional meshes easy.
We accomplish this by insulating your program from the instability and licenses
of the current generation of meshers by providing the ability for
meshing to happen on remote machines using a basic client, server, and worker design.

## Remus Layout ##

Remus is split into five distinct groups: Common, Proto,
Client, Server, and Worker.

Common is a collection of helper classes that usable by any of the other groups.
It contains useful abilities such as MD5 computation, signal catching, process
launching and monitoring, mesh type registration, and a conditional storage class.

Proto contains all the common classes that are used during serialization.
If an object is being sent from the client to the server, it will be in the
proto group.

Client, Server, and Worker are rather self explanatory; they are the interfaces
to the client, server and worker components of Remus.

## Using the Client API ##

The client interface for Remus revolves around the ```remus::Client``` class,
and the communication objects in the ```remus::proto``` namespace.

The client uses a request reply strategy with the server, and each call
on the client will block while it waits for a response from the server.

### Server Connection ###
The first step is to construct a connection to the server. This is done by
constructing a ```ServerConnection``` as follows:

```cpp
//create a default server connection
remus::client::ServerConnection conn;

//create a client object that will connect to the default remus server
remus::Client client(conn);
```

You can also explicitly state the machine name and port:
```cpp
remus::client::ServerConnection conn("meshing_host", 8080);
```

### Communication with Server ###

The communication pattern that Remus uses between the client and
server has a very simple flow. First the client asks
for the set of available workers that match a set of requirements.  From that,
it choses a single worker and crafts a job submission using that worker's
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

Let's put this all together and show to submit a job to the server
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
is sends heartbeat messages to the server.  This allows the server to monitor
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

- Get a Job:
    - To get a single job in a blocking manner:
    ```remus::worker::Job getJob()```
    - To get a single job in a non blocking manner:
    ```cpp
    askForJobs(1)
    remus::worker::Job takePendingJob()
     ```
- Send Job Status:
  ```void updateStatus(const remus::proto::JobStatus& info)```
- Send Job Results:
  ```void returnMeshResults(const remus::proto::JobResult& result)```

Let's put this all together and show how to get a job from the server,
send back some status, and then return a result:

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
remus::proto::JobContent content;
const bool has_data_content = jd.details("data", content);

//extract the data from the content
std::string message;
if(has_data_content)
  {
  message = std::string(content.data(),content.dataSize());
  mesasge += " and ";
  }
message += "Hello Client I am currently in progress";

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

Calling some external command line program is common task for workers.
Remus has built in support for executing processes.

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
to understand you need to generate a Remus worker file. The files is a very
simple JSON file that looks like:

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
ones are insufficient. For example, let's say that you have a meshing process whose input
is actually an AMR dataset and whose output is collection of Voxels. You can
do the following to have Remus support those new types.

```cpp
  #include <remus/common/MeshTypes.h>

  //lets derive a new type from mesh registry
  struct AMRData : remus::meshtypes::MeshTypeBase
  {
  static boost::shared_ptr<MeshTypeBase> create() {
          return boost::shared_ptr<AMRData>(new AMRData()); }
  std::string name() const { return "AMRData"; }
  };

  //lets derive a new type from mesh registry
  struct Voxel : remus::meshtypes::MeshTypeBase
  {
  static boost::shared_ptr<MeshTypeBase> create() {
          return boost::shared_ptr<Voxel>(new Voxel()); }
  std::string name() const { return "Voxel"; }
  };

  remus::common::MeshRegistrar a( (AMRData()) );
  remus::common::MeshRegistrar b( (Voxel()) );
```
At a minimum these new types must be compiled into the Worker that uses them.
The server and client don't need to know about these types, as long
as you are willing to query the server for the supported ```remus::common::MeshIOTypes```.

Now you can use the AMRData and Voxel classes in the worker, like this:

```
remus::common::MeshIOType io_type =
                          remus::common::make_MeshIOType(AMRData(),Voxel());
```
