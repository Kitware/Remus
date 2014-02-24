##Remus: A Guide to the Client Code##


Triangle. Passed a binary encode input uses a custom class to for reflection.
          Also has state for user input.

Tetgen. Passed a file and user flags


Assygen. Passed a directory, and a file name

Cubit. Passed a file

Coregen. Passed a collection of files


In all cases we can split the requirements into two sections;
We have data requirements, and control requirements. Now this makes it easier
to explain what the class should look like.


A: DataTypeRequirements
B: ParameterTypeRequirements.


For example Triangle requires the user to fill out a custom class it defines.
I think the best way to handle this is to state we have two types of requirements,
already known, and runtime choice.


So Triangle has a DataTypeRequirment ( AlreadyKnown ) and no ParameterTypeRequirements
all other classes have a runtime ParameterTypeRequirements.


client side:


MeshRequirements
{
  //some meshers like triangle have no requirements
  bool hasRequirements();

  //format type is independent of the type of requirements
  formatTypes { XML(0), JSON(1), BSON(2), USER(N) };
  formatType wire_type() { return XML; }

  char* getRequirements() const; //return the raw requirements in compa

  //store the mesh input and output type
  remus::common::MeshIOType mesh_type();

  //stores the work name
  std::string workername();

}

JobSubmission( JobData, MeshRequirements )
  {
  //
  }


JobData()
{

  //format type is just for the raw data
  StoreRawData( ..... )
  formatTypes { XML(0), JSON(1), BSON(2), USER(N) };
  formatType wire_type() { return XML; }
  bool encoded_data();

  //files are all read in binary mode, for version 1.0 we can just
  //store file paths and send those
  StoreFiles( files.... )
  workingDir( ? )
}


worker side:

MeshRequirements
{
  std::string name;
  formatTypes .....
  file_location() the requirements file to pass to the client
  binary_data() the requirements in a binary data form

}


JobData()
{
  unsigned int num_files()

  //format type is independent of the type of requirements
  formatTypes { XML(0), JSON(1), BSON(2), USER(N) };
  formatType wire_type() { return XML; }
  bool encoded_data();
  Files
}




This is the basic flow:

1. bool client::canMesh( client::RequiredMeshTypes(inttype, outtype) );
2. std::set < MeshRequirements > client::getMeshRequirements( client::meshTypes(intype, outtype) );
3. client::Job client::submitJob( JobSubmission( JobData(), MeshRequirements() )  );

We have JobSubmission
  - SubmissionInfo holds
    -JobData ( file based, or blob) ( handle multiple files )
    -The desired workers MeshRequrements

  - We strip out the items from the mesh requirements that are need ( meshtypes, worker name)
  - and send those along with the job data to the server in a format that is easy
    to decompose. We are going to need to extend the server to handle this
    new message type


Tasks:
  construct client classes:
    MeshRequirements
    JobData
    JobSubmission

  construct worker classes:
    MeshRequirements on worke
    JobData
    JobSubmission

  change client call api
  change worker to have a name
  change worker to submit mesh requirements when stating it can mesh
  worker mesh requirements should be able to read a file and send the data to the client
      maybe for version 1.0 we can just pass the file handle if server is local
  change worker rw file to store the worker name, the requirements type, and the requirements external file if needed.

  server needs a full rework when saving workers.
  need a new service type
