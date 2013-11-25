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

#ifndef _TriangleInput_h
#define _TriangleInput_h

#include <string>
#include <sstream>
#include <vector>

#include <remus/Job.h>
#include "StreamHelpers.h"

// The TriangleInput class has two purposes.
// For the client side interface it is what the users use to create the
// input data for triangle.
// For the worker it is how we unpack the data from the string that
// remus::Job has

class TriangleInput
{
public:
  //typedefs of the iterator types that this class has
  typedef std::vector<double>::const_iterator double_const_iterator;
  typedef std::vector<double>::iterator double_iterator;

  typedef std::vector<int>::const_iterator int_const_iterator;
  typedef std::vector<int>::iterator int_iterator;

  //create an empty triangle job data class
  TriangleInput();

  TriangleInput(int numPoints,
                int numSegments,
                int numHoles=0,
                int numRegions=0,
                bool perserveSegmentsAndPoints=false);

  //convert a remus::job data into a Triangle input
  TriangleInput(const remus::Job& job);

  bool useMinAngle()const{ return this->MinAngleOn; }
  void setUseMinAngle(bool useMin) {this->MinAngleOn=useMin;}

  double minAngle() const{ return this->MinAngle; }
  void setMinAngle(double angle) { this->MinAngle=angle; }

  bool useMaxArea() const { return this->MaxAreaOn; }
  void setUseMaxArea(bool useMin) { this->MaxAreaOn=useMin; }

  double maxArea() const { return this->MaxArea; }
  void setMaxArea(double area) {this->MaxArea=area;}

  bool preserveBoundaries( ) const { return this->PreserveBoundaries; }
  void setPreserveBoundaries(bool preserve) {this->PreserveBoundaries=preserve;}

  bool setPoint(int index, double x, double y, int nodeId=-1);

  bool setSegment(int index, int pId1, int pId2, int arcId=0);

  bool setHole(int index, double x, double y);

  bool setRegion(int index, double x, double y,
                 double attribute,
                 double max_area);

  //getters to help determine the amount of info we are storing
  int numberOfPoints() const { return this->NumberOfPoints; }
  int numberOfSegments() const { return this->NumberOfSegments; }
  int numberOfHoles() const { return this->NumberOfHoles; }
  int numberOfRegions() const { return this->NumberOfRegions; }
  int numberOfNodes() const { return this->NumberOfNodes; }
  bool perserveSegmentsAndPoints() const
    { return this->PerserveSegmentsAndPoints; }

  //getters to iterate over the vectors. This is to allow
  //the client a more efficient way to write the data, and allow
  //the worker to extract the data easily
  double_const_iterator points_begin() const
    { return this->points.begin(); }
  double_const_iterator points_end() const
    { return this->points.end(); }
  double_iterator points_begin()
    { return this->points.begin(); }
  double_iterator points_end()
    { return this->points.end(); }

  double_const_iterator pointAttributes_begin() const
    { return this->pointAttribute.begin(); }
  double_const_iterator pointAttributes_end() const
    { return this->pointAttribute.end(); }
  double_iterator pointAttributes_begin()
    { return this->pointAttribute.begin(); }
  double_iterator pointAttributes_end()
    { return this->pointAttribute.end(); }

  int_const_iterator segments_begin() const
    { return this->segments.begin(); }
  int_const_iterator segments_end() const
    { return this->segments.end(); }
  int_iterator segments_begin()
    { return this->segments.begin(); }
  int_iterator segments_end()
    { return this->segments.end(); }

  int_const_iterator segmentMarkers_begin() const
    { return this->segmentMarker.begin(); }
  int_const_iterator segmentMarkers_end() const
    { return this->segmentMarker.end(); }
  int_iterator segmentMarkers_begin()
    { return this->segmentMarker.begin(); }
  int_iterator segmentMarkers_end()
    { return this->segmentMarker.end(); }

  double_const_iterator holes_begin() const
    { return this->holes.begin(); }
  double_const_iterator holes_end() const
    { return this->holes.end(); }
  double_iterator holes_begin()
    { return this->holes.begin(); }
  double_iterator holes_end()
    { return this->holes.end(); }

  double_const_iterator regions_begin() const
    { return this->regions.begin(); }
  double_const_iterator regions_end() const
    { return this->regions.end(); }
  double_iterator regions_begin()
    { return this->regions.begin(); }
  double_iterator regions_end()
    { return this->regions.end(); }


  //casting operator to convert this class into a string
  operator std::string(void) const;

private:
  int NumberOfPoints;
  int NumberOfSegments;
  int NumberOfHoles;
  int NumberOfRegions;
  int NumberOfNodes; //tracks number of points with a nodeId > -1

  bool PerserveSegmentsAndPoints;
  bool PreserveBoundaries;
  bool MinAngleOn;
  bool MaxAreaOn;

  double MaxArea;
  double MinAngle;

  std::vector<double> points;
  std::vector<double> pointAttribute;

  std::vector<int> segments;
  std::vector<int> segmentMarker;

  std::vector<double> holes;
  std::vector<double> regions;

};

//----------------------------------------------------------------------------
inline TriangleInput::TriangleInput():
  NumberOfPoints(0),
  NumberOfSegments(0),
  NumberOfHoles(0),
  NumberOfRegions(0),
  NumberOfNodes(0),
  PerserveSegmentsAndPoints(false),
  PreserveBoundaries(true),
  MinAngleOn(false),
  MaxAreaOn(false),
  MaxArea(0.0),
  MinAngle(0.0),
  points(),
  pointAttribute(),
  segments(),
  segmentMarker(),
  holes(),
  regions()
  {
  }

//----------------------------------------------------------------------------
inline TriangleInput::TriangleInput(int numPoints, int numSegments, int numHoles,
                             int numRegions, bool perserveSegmentsAndPoints):
  NumberOfPoints(numPoints),
  NumberOfSegments(numSegments),
  NumberOfHoles(numHoles),
  NumberOfRegions(numRegions),
  NumberOfNodes(0),
  PerserveSegmentsAndPoints(perserveSegmentsAndPoints),
  PreserveBoundaries(true),
  MinAngleOn(false),
  MaxAreaOn(false),
  MaxArea(0.0),
  MinAngle(0.0),
  points(numPoints*2,0.0),
  pointAttribute(),
  segments(numSegments*2, 0),
  segmentMarker(),
  holes(numHoles*2,0.0),
  regions(numRegions*4,0.0)
{
  //doing it this way rather than with a trinary
  if(perserveSegmentsAndPoints)
  {
    this->pointAttribute.resize(numPoints);
    this->segmentMarker.resize(numSegments);
  }
}

//----------------------------------------------------------------------------
inline TriangleInput::TriangleInput(const remus::Job& job)
{
  //convert the from a string back into the class
  std::stringstream buffer(job.details());

  buffer >> this->MinAngleOn;
  buffer >> this->MaxAreaOn;
  buffer >> this->PreserveBoundaries;
  buffer >> this->PerserveSegmentsAndPoints;
  buffer >> this->NumberOfPoints;
  buffer >> this->NumberOfSegments;
  buffer >> this->NumberOfHoles;
  buffer >> this->NumberOfRegions;
  buffer >> this->NumberOfNodes;
  buffer >> this->MaxArea;
  buffer >> this->MinAngle;

  helpers::AllocFromStream(buffer,this->points, this->NumberOfPoints * 2);
  helpers::AllocFromStream(buffer,this->segments, this->NumberOfSegments * 2);
  helpers::AllocFromStream(buffer,this->holes, this->NumberOfHoles * 2);
  helpers::AllocFromStream(buffer,this->regions, this->NumberOfRegions * 4);

  if(this->PerserveSegmentsAndPoints)
    {
    helpers::AllocFromStream(buffer,this->segmentMarker,
                             this->NumberOfSegments);
    helpers::AllocFromStream(buffer,this->pointAttribute,
                             this->NumberOfPoints);
    }
}


//----------------------------------------------------------------------------
inline bool TriangleInput::setPoint(int index, double x, double y, int nodeId)
{
  if (index >= 0 && index < this->NumberOfPoints)
    {
    this->points[index*2]=x;
    this->points[index*2+1]=y;
    if(this->PerserveSegmentsAndPoints)
      {
      if(nodeId != -1)
        {
        this->NumberOfNodes++;
        }
      this->pointAttribute[index] = nodeId;
      }
    return true;
    }
  return false;
}

//----------------------------------------------------------------------------
inline bool TriangleInput::setSegment(int index, int pId1,
                                         int pId2, int arcId)
{
  if (index >= 0 && index < this->NumberOfSegments)
    {
    this->segments[index*2]=pId1;
    this->segments[index*2+1]=pId2;
    if(this->PerserveSegmentsAndPoints)
      {
      this->segmentMarker[index]=arcId;
      }
    return true;
    }
  return false;
}

//----------------------------------------------------------------------------
inline bool TriangleInput::setRegion(int index, double x, double y,
                                        double attribute, double max_area)
{
  if (index >= 0 && index < this->NumberOfRegions)
    {
    this->regions[index*4]=x;
    this->regions[index*4+1]=y;
    this->regions[index*4+2]=attribute;
    this->regions[index*4+3]=max_area;
    return true;
    }
  return false;
}

//----------------------------------------------------------------------------
inline bool TriangleInput::setHole(int index, double x, double y)
{
  if (index >= 0 && index < this->NumberOfHoles)
    {
    this->holes[index*2]=x;
    this->holes[index*2+1]=y;
    return true;
    }
  return false;
}

//------------------------------------------------------------------------------
inline TriangleInput::operator std::string(void) const
{
  //convert the data into a massive string
  std::stringstream buffer;
  buffer << this->MinAngleOn << std::endl;
  buffer << this->MaxAreaOn << std::endl;
  buffer << this->PreserveBoundaries << std::endl;
  buffer << this->PerserveSegmentsAndPoints << std::endl;
  buffer << this->NumberOfPoints << std::endl;
  buffer << this->NumberOfSegments << std::endl;
  buffer << this->NumberOfHoles << std::endl;
  buffer << this->NumberOfRegions << std::endl;
  buffer << this->NumberOfNodes << std::endl;
  buffer << this->MaxArea << std::endl;
  buffer << this->MinAngle << std::endl;

  helpers::WriteToStream(buffer,this->points);
  helpers::WriteToStream(buffer,this->segments);
  helpers::WriteToStream(buffer,this->holes);
  helpers::WriteToStream(buffer,this->regions);

  if(this->PerserveSegmentsAndPoints)
    {
    helpers::WriteToStream(buffer,this->segmentMarker);
    helpers::WriteToStream(buffer,this->pointAttribute);
    }

  buffer << std::endl;
  std::string result = buffer.str();
  return result;
}

#endif
