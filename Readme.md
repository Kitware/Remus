Remus: A Short Users Guide.

# Goal #

Remus goal is to make meshing 2 and 3 dimensional meshes easy.
This is done by insulating your program from the instability and licenses of the
current generation of meshers. It does this by providing the ability for
meshing to happen on remote machines using a basic client, server, worker design.

## Using the Client API ##

The client interface for remus revolves around the ```remus::Client``` class.
The remus client class connects to a single remus server and is uses blocking
communication patterns. All data passed between the client and server is
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

You can also explicitly state the machine name and port explicitly
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


## Developer Notes ##

For how to write a custom client, server, or worker see the
following Readme documents:

- [Client](remus/client/Readme.md)
- [Server](remus/server/Readme.md)
- [Worker](remus/worker/Readme.md)