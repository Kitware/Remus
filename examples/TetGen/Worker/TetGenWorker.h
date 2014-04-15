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

#ifndef TetGenWorker_h
#define TetGenWorker_h

#include <set>

#include <remus/worker/Job.h>
#include <remus/worker/Worker.h>
#include <remus/worker/ServerConnection.h>

//for the info the client is going to send
#include "TetGenInput.h"

// for TetGen itself
#include "tetgen.h"

//simple struct that holds all the arguments to the tetgen process
struct tetgenParameters
{
  //I think the best option is as follows:
  //We create a tetgen input class that stores the input file name
  //and the options needed to mesh that file. We encode that information
  //and send it to the worker, which is directly linked to tetgen.

  //holds the raw client data structures needed for meshing
  TetGenInput meshing_data;

  //holds the tetgen behavior and input mesh
  tetgenbehavior behavior;
  tetgenio in;

  //convert the job details into the parameters needed for tetgen meshing
  tetgenParameters(const remus::worker::Job& job);
  ~tetgenParameters();

  bool valid() const
    {
    return this->meshing_data.valid_file();
    }

  const std::string& inputFilePath() const
    {
    return this->meshing_data.fullFilePath();
    }

};

class TetGenWorker : public remus::worker::Worker
{
public:
  TetGenWorker(remus::worker::ServerConnection const& connection);

  //will get a tetgen job from the remus server
  //and call tetgen
  void meshJob();

protected:

  bool buildTetGenArguments(const tetgenParameters& params,
                            std::string& options) const;
};
#endif
