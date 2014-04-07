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

namespace remus{
namespace server{
namespace detail{

//------------------------------------------------------------------------------
WorkerPool::WorkerInfo::WorkerInfo(const zmq::SocketIdentity& address,
                                   const remus::proto::JobRequirements& reqs):
  WaitingForWork(false),
  Reqs(reqs),
  Address(address),
  expiry(boost::posix_time::second_clock::local_time())
{
  //we give it two heartbeat cycles of lifetime to start
  expiry = expiry + boost::posix_time::seconds(HEARTBEAT_INTERVAL_IN_SEC*2);
}

void WorkerPool::WorkerInfo::refresh()
{
  //we give it two heartbeat cycles to handle packet delay,
  //and workers and server heart-beating on the exact same second
  expiry = boost::posix_time::second_clock::local_time() +
    boost::posix_time::seconds(HEARTBEAT_INTERVAL_IN_SEC*2);
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
  this->Pool.push_back( WorkerPool::WorkerInfo(workerIdentity,reqs) );
  return true;
}

//------------------------------------------------------------------------------
remus::proto::JobRequirementsSet WorkerPool::waitingWorkerRequirements(
                                         remus::common::MeshIOType type) const
{
  remus::proto::JobRequirementsSet validWorkers;
  for(ConstIt i=this->Pool.begin(); i != this->Pool.end(); ++i)
    {
    if( i->Reqs.meshTypes() == type && i->WaitingForWork)
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
    found = ( i->Reqs == reqs && i->WaitingForWork );
    }
  return found;
}

//------------------------------------------------------------------------------
bool WorkerPool::haveWorker(const zmq::SocketIdentity& address) const
{
  bool found = false;
  for(ConstIt i=this->Pool.begin(); !found && i != this->Pool.end(); ++i)
    {
    found = ( i->Address == address );
    }
  return found;
}

//------------------------------------------------------------------------------
bool WorkerPool::readyForWork(const zmq::SocketIdentity& address)
{
  //a worker can be registered multiple times, we need to iterate
  //over the entire vector and find all occurrences of the of the address
  WorkerPool::ReadyForWork pred(address);

  //transform every element that matches the address to be waiting for work
  //need to use the resulting predicate to get the correct result.
  pred = std::for_each(this->Pool.begin(),this->Pool.end(), pred);
  return pred.Count > 0;
}


//------------------------------------------------------------------------------
zmq::SocketIdentity WorkerPool::takeWorker(
                             const remus::proto::JobRequirements& reqs)
{

  bool found = false;
  std::size_t index = 0;
  WorkerPool::WorkerInfo* info;
  for(index=0; index < this->Pool.size(); ++index)
    {
    info = &this->Pool[index];
    found = (info->Reqs == reqs) && (info->WaitingForWork);
    if(found)
      {
      break;
      }
    }

  if(found)
    {
    zmq::SocketIdentity workerIdentity(info->Address);
    this->Pool.erase(this->Pool.begin()+index);
    return workerIdentity;
    }
  return zmq::SocketIdentity();
}

//------------------------------------------------------------------------------
void WorkerPool::purgeDeadWorkers(const boost::posix_time::ptime& time)
{
  WorkerPool::ExpireWorkers pred(time);

  //remove if moves all bad items to end of the vector and returns
  //an iterator to the new end
  It newEnd = std::remove_if(this->Pool.begin(),this->Pool.end(),pred);

  //erase all the elements that remove_if moved to the end
  this->Pool.erase(newEnd,this->Pool.end());
}

//------------------------------------------------------------------------------
void WorkerPool::refreshWorker(const zmq::SocketIdentity& address)
{
  std::for_each(this->Pool.begin(), this->Pool.end(),
                WorkerPool::RefreshWorkers(address));
}
//------------------------------------------------------------------------------
std::set<zmq::SocketIdentity> WorkerPool::livingWorkers() const
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
