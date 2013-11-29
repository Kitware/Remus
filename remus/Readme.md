Remus: A Short Users Guide.


## Using the Client API ##


## Using the Worker API ##

Bugs:

1. We aren't getting all the progress back from workers that we should
   be when using any server,worker, BasicClient combo

Todo:

1. Add a server test where we verify the throughput of the server
   using very basic client / worker calls, we should verify that we can
   handle a certain number of client worker calls no matter what to
   verify we don't regress performance

   This should really be a collection of tests that try to tax the server
   as much as possible, so that we never lose performance.

2. We need to add a JobData.h and JobFile.h to remus::worker, these
   are what we convert the corresponding JobDataRequest and JobFileRequest
   too.

   This is going to require us to change the way Message works

   JobFileRequest needs to be extended to have the following:

   a member string of command args.
   a struct that holds the file information.
      path to file
      read mode of the file
      optional data blob of file
    this struct will need to convert all paths to be absolute
    all relative paths will be evaluated based on the current working directory
    when we determine that the server is not local we have to read the file
    and send it along the wire.
    Do we need to have a threaded client for this to be responsive?

   a string or vector? that holds all the command line arguments that are required
   when reading the file. This can be optional and is used as a way to allow
   JobFileRequest to be used to launch programs easily


2. We need to add a JobDataResult.h and JobFileResult.h to remus::worker, and
   remus::client these allow us to send back a file contents.

   This is going to require us to change the way Response works

   JobFilResult needs to be extended to have the following:

   a struct that holds the file information.
      path to file
      read mode of the file
      optional data blob of file
    this struct will need to convert all paths to be absolute
    all relative paths will be evaluated based on the current working directory
    when we determine that the server is not local we have to read the file
    and send it along the wire.


3. We need to add real logging to  the server, so that it is easier to enable a
   verbose server that will help us figure out concurrency and queueing issues.
   This should be done before threading the server so that it is easier to debug
   issues when moving to a threaded server


4. Look into threading the server. The server is fairly easy to understand
   since it is single threaded. What we would want to do is move the server
   into at MINIMUM three threads, which can use zmq interprocess comms.

   Thread 1. would handle the worker factory, the active job pool,
             the worker pool and the job queue.

   Thread 2. would pool the client connection and send messages to and
             from thread 1.

   Thread 3. would pool the client connection and send messages to and
             from thread 1.

   This design would allow us to use the inherit atomicity of communication
   between threads to keep everything synced. We should look at the heartbeat
   code to make sure it never uses current time to determine what workers and
   jobs to kill.
