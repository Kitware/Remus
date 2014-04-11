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

#include <remus/server/detail/WorkerPool.h>

#include <remus/server/detail/uuidHelper.h>
#include <remus/proto/zmqSocketIdentity.h>

#include <algorithm>

namespace remus{
namespace server{
namespace detail{

//------------------------------------------------------------------------------
WorkerPool::WorkerInfo::WorkerInfo(const zmq::SocketIdentity& address,
                                   const remus::proto::JobRequirements& reqs):
  NumberOfDesiredJobs(0),
  Reqs(reqs),
  Address(address),
  IsAlive(true)
{
}

//------------------------------------------------------------------------------
WorkerPool::WorkerPool():
  Pool()
{

}

//------------------------------------------------------------------------------
bool WorkerPool::addWorker(zmq::SocketIdentity workerIdentity,
                           const remus::proto::JobRequirements& reqs)
{
  if(!this->haveWorker(workerIdentity,reqs))
    {
    this->Pool.push_back( WorkerPool::WorkerInfo(workerIdentity,reqs) );
    }
  return true;
}

//------------------------------------------------------------------------------
remus::proto::JobRequirementsSet WorkerPool::waitingWorkerRequirements(
                                         remus::common::MeshIOType type) const
{
  remus::proto::JobRequirementsSet validWorkers;
  for(ConstIt i=this->Pool.begin(); i != this->Pool.end(); ++i)
    {
    if( i->Reqs.meshTypes() == type && i->isWaitingForWork() )
      { validWorkers.insert(i->Reqs); }
    }
  return validWorkers;
}

//------------------------------------------------------------------------------
bool WorkerPool::haveWaitingWorker(
                           const remus::proto::JobRequirements& reqs) const
{
  bool found = false;
  for(ConstIt i=this->Pool.begin(); !found && i != this->Pool.end(); ++i)
    {
    found = ( i->Reqs == reqs && i->isWaitingForWork() );
    }
  return found;
}

//------------------------------------------------------------------------------
bool WorkerPool::haveWorker(const zmq::SocketIdentity& address,
                            const remus::proto::JobRequirements& reqs) const
{
  bool found = false;
  for(ConstIt i=this->Pool.begin(); !found && i != this->Pool.end(); ++i)
    {
    found = ( i->Address == address && i->Reqs == reqs );
    }
  return found;
}

//------------------------------------------------------------------------------
bool WorkerPool::readyForWork(const zmq::SocketIdentity& address,
                              const remus::proto::JobRequirements& reqs)
{
  //a worker can be registered multiple times, we need to iterate
  //over the entire vector and find the correct address and reqs that match
  //transform every element that matches the address and reqs to be
  //waiting for work, If the worker is already waiting for work we increase
  //the number of jobs it is waiting to take.
  int count = 0;
   for(It i=this->Pool.begin(); i != this->Pool.end(); ++i)
    {
    if(i->Address == address && i->Reqs == reqs)
      {
      i->IsAlive = true; //mark the worker alive if it wasn't already
      i->addJob();
      ++count;
      }
    }
  return (count > 0);
}


//------------------------------------------------------------------------------
zmq::SocketIdentity WorkerPool::takeWorker(
                             const remus::proto::JobRequirements& reqs)
{
  bool found = false;
  It i;
  for(It i=this->Pool.begin(); !found && i != this->Pool.end(); ++i)
    {
    found = (i->Reqs == reqs) && (i->isWaitingForWork());
    }

  zmq::SocketIdentity workerIdentity;

  if(found)
    {
    //take the worker id as it matches the reqs
    workerIdentity = zmq::SocketIdentity(i->Address);
    i->takesJob();

    //now that the worker has taken the job, we move him to the back of
    //the vector so he is the last worker to take a job of that type again,
    //this allows us to handle multiple workers taking jobs
    std::rotate(this->Pool.begin(), this->Pool.begin() + 1,this->Pool.end());
    }

  return workerIdentity;
}

//------------------------------------------------------------------------------
void WorkerPool::purgeDeadWorkers(remus::server::detail::SocketMonitor monitor)
{
  //Remove all workers that we know are really dead
  WorkerPool::DeadWorkers dead(monitor);

  //remove if moves all bad items to end of the vector and returns
  //an iterator to the new end. Remove if is easiest way to remove from middle
  It newEnd = std::remove_if(this->Pool.begin(),this->Pool.end(),dead);

  for(It i=this->Pool.begin(); i != newEnd; ++i)
    {
    i->IsAlive = !monitor.isUnresponsive(i->Address);
    }

  //erase all the dead workers to free up space
  this->Pool.erase(newEnd,this->Pool.end());
}

//------------------------------------------------------------------------------
std::set<zmq::SocketIdentity> WorkerPool::allWorkers() const
{
  std::set<zmq::SocketIdentity> workerAddresses;
  for(ConstIt i=this->Pool.begin(); i != this->Pool.end(); ++i)
    {
    workerAddresses.insert(i->Address);
    }
  return workerAddresses;
}

}
}
}
