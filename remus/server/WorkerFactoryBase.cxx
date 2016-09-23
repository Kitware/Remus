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

#include <remus/server/WorkerFactoryBase.h>

#include <remus/server/ServerPorts.h>


namespace remus{
namespace server{

//----------------------------------------------------------------------------
WorkerFactoryBase::WorkerFactoryBase():
  MaxWorkers(1),
  WorkerEndpoint()
{

}

//----------------------------------------------------------------------------
WorkerFactoryBase::~WorkerFactoryBase()
{

}

//----------------------------------------------------------------------------
void WorkerFactoryBase::portForWorkersToUse(const remus::server::PortConnection& port)
{
  this->WorkerEndpoint = port.endpoint();
}

}

}
