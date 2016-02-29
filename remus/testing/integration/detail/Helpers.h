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
//  Copyright 2012 Sandia Corporation.
//  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
//  the U.S. Government retains certain rights in this software.
//
//=============================================================================
#ifndef remus_testing_integration_detail_Helpers_h
#define remus_testing_integration_detail_Helpers_h

#include <remus/common/SleepFor.h>

#include <remus/client/Client.h>
#include <remus/client/ServerConnection.h>

#include <remus/testing/Testing.h>

namespace remus {
namespace testing {
namespace integration {
namespace detail {

//------------------------------------------------------------------------------
inline void verify_job_status(remus::proto::Job  job,
                              boost::shared_ptr<remus::Client> client,
                              remus::STATUS_TYPE statusType)
{
  using namespace remus::proto;

  bool valid_status = false;
  const int tries = 4;
  for(int i=0; i < tries && !valid_status; ++i)
    { //try up to 4 times to handle really slow test machines
    remus::common::SleepForMillisec(250);
    JobStatus currentStatus = client->jobStatus(job);
    valid_status = (currentStatus.status() == statusType);
    }
  REMUS_ASSERT(valid_status)
}

//------------------------------------------------------------------------------
inline boost::shared_ptr<remus::Client> make_Client( const remus::server::ServerPorts& ports,
                                                     bool share_context = false)
{
  remus::client::ServerConnection conn =
              remus::client::make_ServerConnection(ports.client().endpoint());
  if(share_context)
    {
    conn.context(ports.context());
    }

  boost::shared_ptr<remus::Client> c(new remus::client::Client(conn));
  return c;
}

//------------------------------------------------------------------------------
inline
boost::shared_ptr<remus::Worker> make_Worker( const remus::server::ServerPorts& ports,
                                              const remus::common::MeshIOType& io_type,
                                              const std::string& name,
                                              bool share_context = false )
{
  using namespace remus::meshtypes;
  using namespace remus::proto;

  //because the workers use inproc they need to share the same context
  remus::worker::ServerConnection conn =
              remus::worker::make_ServerConnection(ports.worker().endpoint());
  if(share_context)
    {
    conn.context(ports.context());
    }

  JobRequirements requirements = make_JobRequirements(io_type, name, "");
  boost::shared_ptr<remus::Worker> w(new remus::Worker(requirements,conn));
  return w;
}

}
}
}
}


#endif