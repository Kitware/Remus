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

#ifndef remus_examples_TriangleWorker_h
#define remus_examples_TriangleWorker_h

#include <remus/worker/Job.h>
#include <remus/worker/ServerConnection.h>
#include <remus/worker/Worker.h>

#include "TriangleInput.h"

// for Triangle
#ifndef ANSI_DECLARATORS
#define ANSI_DECLARATORS
#define VOID void
#endif

#ifndef REAL
#ifdef SINGLE
#define REAL float
#else                           /* not SINGLE */
#define REAL double
#endif                          /* not SINGLE */
#endif
extern "C"
{
#include "triangle.h"
}
//undef triangle defines
#undef ANSI_DECLARATORS
#undef VOID
#undef TRIANGLE_REAL
// END for Triangle

//simple struct that holds all the data structures needed by triangle
struct triangleParameters
{
  //holds the raw triangle data structures needed for meshing
  TriangleInput meshing_data;
  struct triangulateio in;
  struct triangulateio out;

  //convert the job details into the parameters needed for triangle meshing
  //we use a TriangleInput class for the actual conversion
  triangleParameters(const remus::worker::Job& job);
  ~triangleParameters();

  bool valid() const
    {
    return this->meshing_data.numberOfPoints() >= 3 &&
           this->meshing_data.numberOfSegments() >= 3;
    }
};

class TriangleWorker : public remus::worker::Worker
{
public:
  TriangleWorker(remus::worker::ServerConnection const& connection);

  //will get a triangle job from the remus server
  //and will call triangle inside a its own thread to mesh the job
  void meshJob();

protected:

  bool buildTriangleArguments(const triangleParameters& params,
                              std::string& options) const;

  void jobFailed(const remus::worker::Job& job);
};
#endif
