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

namespace remus {
namespace testing {
namespace integration {
namespace detail {

//------------------------------------------------------------------------------
static void verify_job_status(remus::proto::Job  job,
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

}
}
}
}


#endif