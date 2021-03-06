#include <iostream>

#include <remus/client/Client.h>
#include <remus/client/ServerConnection.h>

#include "TriangleInput.h"
#include "TriangleResult.h"

int main (int argc, char* argv[])
{
  remus::client::ServerConnection connection;
  if(argc>=2)
    {
    //let the server connection handle parsing the command arguments
    connection = remus::client::make_ServerConnection(std::string(argv[1]));
    }

  //create a client object that will connect to the default remus server
  remus::Client c(connection);


  //construct a very basic triangle dataset
  //7 points, 7 segments, 1 hole
  TriangleInput input_data(7,7,1);

  //set up lines
  input_data.setSegment(0,0,1); //start of outside box
  input_data.setSegment(1,1,2);
  input_data.setSegment(2,2,3);
  input_data.setSegment(3,3,0); //end of outside box
  input_data.setSegment(4,4,5); //start of hole
  input_data.setSegment(5,5,6);
  input_data.setSegment(6,6,4); //end of triangle shaped hole

  //setup the points
  input_data.setPoint(0, 0, 10);    //top left box
  input_data.setPoint(1, 10, 10);   //top right box
  input_data.setPoint(2, 10, 0);    //bottom right box
  input_data.setPoint(3, 0, 0);     //bottom left box

  input_data.setPoint(4, 3, 3);
  input_data.setPoint(5, 4, 5);
  input_data.setPoint(6, 5, 3);

  input_data.setHole(0, 4, 4); //hole in the middle of triangle
  remus::proto::JobContent content =
                          remus::proto::make_JobContent(input_data);

  //we create a basic job request for a mesh2d job
  remus::common::MeshIOType mtype( (remus::meshtypes::Edges()),
                                    (remus::meshtypes::Mesh2D()) );
  remus::proto::JobRequirements request =
                          remus::proto::make_JobRequirements( mtype,
                                                               "TriangleWorker",
                                                               "");

  if(c.canMesh(mtype))
    {
    remus::proto::JobSubmission sub(request);
    sub["data"]=content;

    remus::proto::Job job = c.submitJob(sub);
    remus::proto::JobStatus jobState = c.jobStatus(job);

    //wait while the job is running
    while(jobState.good())
      {
      jobState = c.jobStatus(job);
      };

    if(jobState.finished())
      {
      remus::proto::JobResult result = c.retrieveResults(job);
      TriangleResult triangle_data(result);

      std::cout << triangle_data.points.size() << std::endl;
      std::cout << triangle_data.lines.size() << std::endl;
      std::cout << triangle_data.triangles.size() << std::endl;
      }
    }
  return 0;
}