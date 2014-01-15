##Remus: A Guide to the Client Code##


The client API resolves


What is our basic flow?:

 request types (3d, 2d, 1d, etc support)
 request requirements for type
 submit job
 job status
 job results

If this is our basic flow what would the methods look like?:

1. bool client::canMesh( client::MeshTypes(inttype, outtype) );

2. std::set < MeshRequirements > client::getMeshRequirements( client::MeshTypes(intype, outtype) );

   - How do we describe MeshRequirements to make it distinct on the way back
     my concern is that we can have multiple requirements types that
     need
      a. MeshTypes(inttype, outtype)
      b. name????

3.

How can we express the packaging of the job, and the requirements
where each MeshRequirements holds the worker id that the reqs apply to?

std::vector< MeshRequirements > DetermineRequirements( JobType )

We have JobSubmissionInfo
  - SubmissionInfo holds
    -JobData ( file based, or blob) ( handle multiple files )
    -JobRequrements
    -Filled out Requirements structure?
How does the MeshRequirements and so forth deal with templating?
we have so requirements that are going to require multiple files be sent
to the worker?
