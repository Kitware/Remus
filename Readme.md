# Remus: A Short Users Guide. #

Remus goal is to make meshing 2 and 3 dimensional meshes easy.
This is done by insulating your program from the instability and licenses of the
current generation of meshers. It does this by providing the ability for
meshing to happen on remote machines using a basic client, server, worker design.

## Using the Client API ##

The client interface for remus revolves around the ```remus::Client``` class.
The client uses a request reply strategy with the server, and each call
on the client will block while it waits for a response from the server.
All data passed between the client and server is
wrapped in ```remus::client::Job*``` classes.

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
server is has a very simple flow. The process can be broken
into 4 major steps. Those steps are:

- Query for support of a job
  ```bool canMesh( remus::client::JobRequest )```
- Submit a job
  ```remus::client::Job submitJob( remus::client::JobRequest )```
- Monitor the job
  ```remus::client::JobStatus jobStatus( remus::client::Job )```
- Retrieve the results
  ```remus::client::JobResult retrieveResults( remus::client::Job )```

Lets put this all together and show to submit a job to the server
and retrieve the results.

```cpp
remus::client::JobRequest request(remus::RAW_EDGES,
                                  remus::MESH2D,
                                  "Test Data");
if(client.canMesh(request))
  {
  remus::client::Job j = client.submitJob(request);
  remus::client::JobStatus jobState = client.jobStatus(j);
  while(jobState.good())
    {
    jobState = client.jobStatus(j);
    }

  //finished returns true if the worker sent a result to the server
  //if the meshing crashed or failed finished returns false
  if(jobState.finished())
    {
    remus::client::JobResult result = client.retrieveResults(j);
    }
  }
```

## Using the Worker API ##

The worker interface for remus revolves around the ```remus::Worker``` class.
The worker uses an asynchronous strategy where the worker pushes information to the
server and doesn't wait for a response. Between these messages the worker
is sending heartbeat messages to the server, these allow the server to monitor
workers and be able to inform client when workers crash. The only worker
message that is blocking is the ```getJob``` method since it waits on the
server to send back a job to process. All data passed between the worker and server is
wrapped in ```remus::worker::Job*``` classes.


### Server Connection ###
The first step is to construct a connection to the server. This is done by
constructing a ```ServerConnection```.

```cpp
//create a default server connection
remus::worker::ServerConnection conn;

//create a worker object that will connect to the default remus server
//with a input model type and an output mesh3d type
remus::Worker worker(remus::common::MeshIOType(remus::MODEL,remus::MESH3D),
                     conn);
```

You can also explicitly state the machine name and port
```cpp
remus::worker::ServerConnection conn("meshing_host", 8080);
```

### Communication with Server ###

The communication flow between the worker and server is very simple.
The process can be broken into 3 major steps. Those steps are:

- Get a Job
  ```remus::worker::Job getJob()```
- Send Job Status
  ```void updateStatus(const remus::worker::JobStatus& info)```
- Send Job Results
  ```void returnMeshResults(const remus::worker::JobResult& result)```

Lets put this all together and show how to get a job from the server,
send back some status, and than a result

```cpp

remus::worker::Job j = worker.getJob();
remus::worker::JobStatus status(j.id(),remus::IN_PROGRESS);

for(int progress=0; progress < 100; progress+=10)
  {
  status.Progress.setValue(progress+1);
  status.Progress.setMessage("Example Message With Random Content");
  worker.updateStatus(status);
  }

remus::worker::JobResult results(j.id(),"HELLO CLIENT");
worker.returnMeshResults(results);

```

### Calling an External Program ###

A common occurrence for a worker is the need to call some external
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



## Developer Notes ##

For how to write a custom client, server, or worker see the
following Readme documents:

- [Client](remus/client/Readme.md)
- [Server](remus/server/Readme.md)
- [Worker](remus/worker/Readme.md)