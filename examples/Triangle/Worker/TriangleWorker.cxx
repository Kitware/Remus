//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//=============================================================================


#include "TriangleWorker.h"
#include "TriangleInput.h"
#include "TriangleResult.h"

#include <cstdlib>
#include <sstream>
#include <iostream>
#include <fstream>

namespace
{

void release_triangle_data(triangulateio* data)
{
  free( data->pointlist );
  free( data->pointattributelist );
  free( data->pointmarkerlist );
  free( data->trianglelist );
  free( data->triangleattributelist );
  free( data->trianglearealist );
  free( data->neighborlist );
  free( data->segmentlist );
  free( data->segmentmarkerlist );
  free( data->holelist );
  free( data->regionlist );
  free( data->edgelist );
  free( data->edgemarkerlist );
  free( data->normlist );

  return;
}

}

//----------------------------------------------------------------------------
triangleParameters::triangleParameters(const remus::worker::Job& job):
  meshing_data(job)
{
  //first we init the intput and output data structures to be empty
  std::memset(&this->in,  0, sizeof(triangulateio));
  std::memset(&this->out, 0, sizeof(triangulateio));

  //make sure the input variable has all the right number of elements
  this->in.numberofsegments = meshing_data.numberOfSegments();
  this->in.numberofpoints = meshing_data.numberOfPoints();
  this->in.numberofholes = meshing_data.numberOfHoles();
  this->in.numberofregions = meshing_data.numberOfRegions();
  this->in.numberoftriangleattributes = meshing_data.numberOfRegions() > 0;

  //we are now going to modify the triangle input struct to point
  //to memory that we have allocated with TriangleInput
  this->in.pointlist = &(*meshing_data.points_begin());
  this->in.segmentlist = &(*meshing_data.segments_begin());
  if (meshing_data.numberOfHoles() > 0)
    {
    this->in.holelist = &(*meshing_data.holes_begin());
    }
  if (meshing_data.numberOfRegions() > 0)
    {
    this->in.regionlist = &(*meshing_data.regions_begin());
    }

  if (meshing_data.perserveSegmentsAndPoints())
    {
    this->in.segmentmarkerlist = &(*meshing_data.segmentMarkers_begin());
    this->in.pointattributelist = &(*meshing_data.pointAttributes_begin());
    }
}

//----------------------------------------------------------------------------
triangleParameters::~triangleParameters()
{
  //not only do we share information between the meshing_data and the
  //input triangle data, triangle itself will share certain information between
  //the in and out structs.

  //so to get this all to work properly we need to set certain
  //points in the in and out triangle data structures to NULL
  bool pointListShared = (this->in.pointlist == this->out.pointlist);
  bool segmentListShared = (this->in.segmentlist == this->out.segmentlist);
  bool segmentMarkerListShared =
                (this->in.segmentmarkerlist == this->out.segmentmarkerlist);
  bool holeListShared = (this->in.holelist == this->out.holelist);
  bool regionListShared = (this->in.regionlist == this->out.regionlist);
  bool pointAttributeListShared =
                (this->in.pointattributelist == this->out.pointattributelist);

  //set these all to null since they will
  //be release by the meshing_data object when it goes out of scope
  this->in.pointlist = NULL;
  this->in.segmentlist = NULL;
  this->in.holelist = NULL;
  this->in.regionlist = NULL;
  this->in.segmentmarkerlist = NULL;
  this->in.pointattributelist = NULL;

  if (pointListShared)
    { this->out.pointlist=NULL; }
  if (segmentListShared)
    { this->out.segmentlist=NULL; }
  if (segmentMarkerListShared)
    { this->out.segmentmarkerlist=NULL; }
  if (holeListShared)
    {  this->out.holelist=NULL; }
  if (regionListShared)
    { this->out.regionlist=NULL; }
  if (pointAttributeListShared)
    { this->out.pointattributelist=NULL; }

  release_triangle_data(&this->in);
  release_triangle_data(&this->out);
}

//----------------------------------------------------------------------------
TriangleWorker::TriangleWorker(
                     remus::worker::ServerConnection const& connection):
  Worker(
    remus::proto::make_JobRequirements(
      remus::common::make_MeshIOType(remus::meshtypes::Edges(),
                                     remus::meshtypes::Mesh2D()),
      "TriangleWorker",
      ""),
  connection)
{
}

//----------------------------------------------------------------------------
bool TriangleWorker::buildTriangleArguments(const triangleParameters &params,
                                            std::string &options) const
{
  bool valid = true;
  double value = 0;

  std::stringstream buffer;
  const TriangleInput& input = params.meshing_data;

  buffer << "p";//generate a planar straight line graph
  buffer << "z";//use 0 based indexing

  if(input.verbose())
    {
    buffer << "V";//enable verbose output
    }
  if(input.useMaxArea() && input.numberOfRegions() == 0)
    {
    value = input.maxArea();
    if (value < 0.0)
      {
      //invalid area constraint
      return false;
      }
    buffer << "a" << std::fixed << value;
    }
  else if(input.numberOfRegions() > 0)
    {
    //To get triangle region attributes use the "A" flag which does not play
    //nice with the "a" flag
    buffer << "A";
    }
  if (input.useMinAngle())
    {
    value = input.minAngle();
    if (value < 0.0 || value > 33.)
      {
      //invalid area constraint
      return false;
      }
    buffer << "q" << std::fixed << value;
    }
  if (input.preserveBoundaries())
    {
    buffer << "Y";//preserve boundaries
    }

  //assign
  options = buffer.str();
  return valid;
}

//----------------------------------------------------------------------------
void TriangleWorker::meshJob()
{
  remus::worker::Job j = this->Worker.getJob();

  //extract the parameters of the job to launch, including the raw edges
  triangleParameters parms(j);

  bool canLaunchTriangle = false;

  std::string options;
  canLaunchTriangle = parms.valid();
  canLaunchTriangle = canLaunchTriangle && this->buildTriangleArguments(parms,
                                                                      options);
  if (!canLaunchTriangle)
    {
    this->jobFailed(j);
    return;
    }

  triangulate(const_cast<char*>(options.c_str()),
              &parms.in,
              &parms.out,
              static_cast<struct triangulateio*>(NULL)) ;

  //the default triangle operation really can't fail. If it hits
  //a point where it can't mesh it will actually call exit which will
  //kill this worker, which will cause the remus server to mark the job
  //as failed.

  //if you have a modified version of triangle that returns op codes
  //instead of exit you will need to modify this call to handle
  //that use case

  //send the data back to the server

  //pass the TriangleResult class the meshing input data
  //and the output triangle data. The real trick is that
  //we are going template the triangle io parameter
  //so we don't need to link to triangle on the client
  //when we transform the string back to the TriangleResult class
  remus::proto::JobResult results = remus::proto::make_JobResult(j.id(),
                       TriangleResult::ToString(parms.meshing_data,parms.out));
  this->Worker.returnResult(results);

  return;
}

//----------------------------------------------------------------------------
void TriangleWorker::jobFailed(const remus::worker::Job& job)
{
  remus::proto::JobStatus status(job.id(),remus::FAILED);
  this->Worker.updateStatus(status);

  return;
}
