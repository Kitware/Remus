## Using the Client API ##

The client interface for Remus revolves around the ```remus::Client``` class,
and the communication objects in the ```remus::proto``` namespace.

The client uses a request reply strategy with the server, and each call on the
client will block while it waits for a response from the server.

The communication pattern that Remus uses between the client and server has a
simple flow. Initial the client asks for the set of available workers that match
either a given ```MeshIOType``` or ```JobRequirements```. From that it decides
which worker it wants and submits a job submission using that worker's requirements.

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

### Client Server Connection ###

The server that the remus client connects to is determined by the ```ServerConnection```
that is provided at construction of the client. The ```ServerConnection``` by
default connects to a local remus server using tcp-ip. You can request a custom
location by doing one of the following:

```cpp
//You can explicitly state the machine name and port with tcp-ip by using one of the following:
remus::client::ServerConnection conn("meshing_host", 8080);
remus::client::ServerConnection conn_other = remus::client::make_ServerConnection("tcp://74.125.30.106:82");

//You can also request a custom inproc or ipc connection by doing:
remus::client::ServerConnection sc_inproc = remus::client::make_ServerConnection("inproc://server");
remus::client::ServerConnection sc_ipc = remus::client::make_ServerConnection("ipc://server");
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
