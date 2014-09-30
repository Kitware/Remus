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


#include "TetGenWorker.h"
#include "TetGenInput.h"
#include "TetGenResult.h"

#include <cstdlib>
#include <sstream>
#include <iostream>
#include <fstream>

namespace detail
{
//----------------------------------------------------------------------------
inline void copy_into_c_string(const std::string& input, char* output)
{
std::copy(input.begin(), input.end(), output );
output[input.size()]='\0';
}
}

//----------------------------------------------------------------------------
tetgenParameters::tetgenParameters(const remus::worker::Job& job):
  meshing_data(job)
{

}

//----------------------------------------------------------------------------
tetgenParameters::~tetgenParameters()
{

}

//----------------------------------------------------------------------------
TetGenWorker::TetGenWorker(remus::worker::ServerConnection const& connection):
  remus::worker::Worker(
    remus::proto::make_JobRequirements(
      remus::common::make_MeshIOType(remus::meshtypes::PiecewiseLinearComplex(),
                                     remus::meshtypes::Mesh3D()),
      "TetGenWorker",
      ""),
    connection)
{
}


//----------------------------------------------------------------------------
bool TetGenWorker::buildTetGenArguments(const tetgenParameters &params,
                                            std::string &options) const
{
  //pzMVYAa are the falgs we want to support
  std::stringstream buffer;
  const TetGenInput& input = params.meshing_data;

  buffer << "-";
  buffer << "p";//generate a planar straight line graph
  buffer << "z";//use zero based indexing

  if(!input.mergeCoplanarFacets())
    {
    buffer << "M";
    }

  if(input.mergeCoplanarFacets() &&
     input.useMergeCoplanarFacetsTolerance())
    {
    const double value =  input.customMergeCoplanarFacetsTolerance();
    if (value < 0.0)
      {
      return false;
      }
    buffer << "T" << value;
    }

  if(input.verbose())
    {
    buffer << "V";
    }

  if(input.perserveSurfaceMesh())
    {
    buffer << "Y";
    }

  if(input.haveRegionAtributes())
    {
    buffer << "A";
    }

  if(input.useVolumeConstraint())
    {
    const double value = input.volumeConstraint();
    if (value < 0.0)
      {
      return false;
      }
    buffer << "a" << value;
    }

  //assign
  options = buffer.str();
  return true;
}

//----------------------------------------------------------------------------
void TetGenWorker::meshJob()
{
  remus::worker::Job j = this->getJob();

  //extract the parameters of the job to launch, including the raw edges
  tetgenParameters parms(j);

  bool canLaunchTetGen = false;

  std::string options;
  canLaunchTetGen = parms.valid();
  canLaunchTetGen = canLaunchTetGen && this->buildTetGenArguments(parms,
                                                                  options);
  if (!canLaunchTetGen)
    {
    remus::proto::JobStatus status(j.id(),remus::FAILED);
    this->updateStatus(status);
    return;
    }

  std::string file_path = parms.inputFilePath();

  //I love passing by char** instead of const char** :(
  char** fake_argv = new char*[2];
  fake_argv[0] = new char[options.size()+1];
  fake_argv[1] = new char[file_path.size()+1];
  detail::copy_into_c_string(options,fake_argv[0]);
  detail::copy_into_c_string(file_path,fake_argv[1]);

  //parse the command string and file name we just made up
  parms.behavior.parse_commandline(2,fake_argv);

  //clean up my memory, lets try not to leak
  delete[] fake_argv;

  parms.in.load_plc(parms.behavior.infilename,
                    static_cast<int>(parms.behavior.object) );

  tetrahedralize(&parms.behavior, &parms.in, NULL, NULL, NULL);

  //we can now look at the output folder and determine the results?
  //or do we
  //send the data back to the server
  const TetGenResult::OutputFileType fileType =
    static_cast<TetGenResult::OutputFileType>(parms.behavior.object);
  TetGenResult tetResults(parms.behavior.outfilename, fileType);

  remus::proto::JobResult results = remus::proto::make_JobResult(j.id(),
                                                                  tetResults);
  this->returnResult(results);
  return;
}
