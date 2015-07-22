## Remus: A Remote Mesh/Model Service Framework

[![Imgur](http://i.imgur.com/8zsK3vl.png?2)](http://open.cdash.org/index.php?project=Remus)

Remus' goal is to make meshing 2 and 3 dimensional meshes easy.
We accomplish this by insulating your program from the instability and licenses
of the current generation of meshers by providing the ability for
meshing to happen on remote machines using a basic client, server, and worker design.

## Building Remus

In order to build Remus you must have

+ A modern C++ compiler (gcc, clang, or VS),
+ [CMake](http://cmake.org) 2.8.11 or newer, and
+ [Boost](http://boost.org) 1.50.0 or newer, and
+ [ZMQ](http://zeromq.org/) 2.X or newer.


Remus provides a superbuild branch which can be used to easily build the Boost
and ZMQ dependencies.

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

## Documentation ##
* [Client Documentation][]
* [Worker Documentation][]
* [Server Documentation][]
* [Constructing a Remus Worker File][]
* [Registering a New Mesh Type][]


[Client Documentation]: remus/client/Readme.md
[Worker Documentation]: remus/worker/Readme.md
[Server Documentation]: remus/server/Readme.md


[Constructing a Remus Worker File]: remus/worker/Readme.md#constructing-a-remus-worker-file
[Registering a New Mesh Type]: remus/client/Readme.md#register-a-new-mesh-type
