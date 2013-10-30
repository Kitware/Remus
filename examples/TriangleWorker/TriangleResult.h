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

#ifndef _TriangleResult_h
#define _TriangleResult_h

#include <string>
#include <sstream>
#include <vector>

#include <remus/Job.h>

#include "TriangleInput.h"
#include "StreamHelpers.h"

// The TriangleResult class has two purposes.
// For the client side interface it is what the users gets back after
// request a meshing job with the triangle input data
// For the worker it is how we pack the data from the triangle process
// and send it back inside a remus::JobResult

class TriangleResult
{
public:
  TriangleResult(const remus::JobResult& result)
  {
  std::stringstream buffer(result.Data);
  buffer >> this->NumberOfPoints;
  buffer >> this->NumberOfLines;
  buffer >> this->NumberOfTriangles;

  bool perserveSegmentsAndPoints=false;
  int numberOfRegions=0;
  buffer >> perserveSegmentsAndPoints;
  buffer >> numberOfRegions;

  helpers::AllocFromStream(buffer,this->points, this->NumberOfPoints * 2);
  helpers::AllocFromStream(buffer,this->lines, this->NumberOfLines * 2);
  helpers::AllocFromStream(buffer,this->triangles, this->NumberOfTriangles * 3);

  //have to store this state option
  if (perserveSegmentsAndPoints)
    {
    helpers::AllocFromStream(buffer,this->pointAttribute,this->NumberOfPoints);
    helpers::AllocFromStream(buffer,this->lineMarkers,this->NumberOfLines);
    }

  //have to store this state option
  if (numberOfRegions)
    {
    helpers::AllocFromStream(buffer,this->triangleAttributes,
                             this->NumberOfTriangles);
    }
  }


  template<class TriangleDataStructure>
  static std::string ToString(const TriangleInput& input,
                              const TriangleDataStructure& data);
  int NumberOfPoints;
  int NumberOfLines;
  int NumberOfTriangles;

  std::vector<double> points;
  std::vector<double> pointAttribute;

  std::vector<int> lines;
  std::vector<int> lineMarkers;

  std::vector<int> triangles;
  std::vector<double> triangleAttributes;
};


//----------------------------------------------------------------------------
template<class TriangleDataStructure>
std::string TriangleResult::ToString(const TriangleInput& input,
                                     const TriangleDataStructure& data)
{
  std::stringstream buffer;
  buffer << data.numberofpoints << std::endl;
  buffer << data.numberofsegments << std::endl;
  buffer << data.numberoftriangles << std::endl;

  buffer << input.perserveSegmentsAndPoints() << std::endl;
  buffer << input.numberOfRegions() << std::endl;

  helpers::WriteToStream(buffer, data.pointlist, data.numberofpoints*2 );
  helpers::WriteToStream(buffer, data.segmentlist, data.numberofsegments*2);
  helpers::WriteToStream(buffer, data.trianglelist, data.numberoftriangles*3);

  if (input.perserveSegmentsAndPoints())
    {
    helpers::WriteToStream(buffer,data.pointattributelist,
                           data.numberofpoints);
    helpers::WriteToStream(buffer,data.segmentmarkerlist,
                           data.numberofsegments);
    }

  if(input.numberOfRegions() > 0)
    {
    helpers::WriteToStream(buffer,data.triangleattributelist,
                           data.numberoftriangles);
    }

  buffer << std::endl;
  return buffer.str();
}
#endif
