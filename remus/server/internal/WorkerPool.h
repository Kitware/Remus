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

#ifndef __remus_server_internal_ActiveJobState_h
#define __remus_server_internal_ActiveJobState_h

#include <boost/date_time/posix_time/posix_time.hpp>

#include <remus/server/internal/uuidHelper.h>
#include <remus/common/zmqHelper.h>

#include <string>
#include <set>
#include <vector>

namespace remus{
namespace server{
namespace internal{

class WorkerPool
{
  public:
    WorkerPool():Pool(){}

    inline bool addWorker(zmq::socketIdentity workerIdentity,
                          const remus::common::MeshIOType& type);

    //do we have any worker waiting to take this type of job
    inline bool haveWaitingWorker(const remus::common::MeshIOType& type) const;

    //do we have a worker with this address?
    inline bool haveWorker(const zmq::socketIdentity& address) const;

    //mark a worker with the given address ready to take a job.
    //returns false if a worker with that address wasn't found
    inline bool readyForWork(const zmq::socketIdentity& address);

    //returns the worker address and removes the worker from the pool
    inline zmq::socketIdentity takeWorker(const remus::common::MeshIOType& type);

    //remove all workers that haven't responded inside the heartbeat time
    inline void purgeDeadWorkers(const boost::posix_time::ptime& time);

    //keep all workers alive that responded inside the heartbeat time
    inline void refreshWorker(const zmq::socketIdentity& address);

    //return the socket identity of all living workers
    inline std::set<zmq::socketIdentity> livingWorkers() const;

private:
    struct WorkerInfo
    {
      bool WaitingForWork;
      remus::common::MeshIOType MType;
      zmq::socketIdentity Address;
      boost::posix_time::ptime expiry; //after this time the job should be purged

      WorkerInfo(const zmq::socketIdentity& address, const remus::common::MeshIOType type):
        WaitingForWork(false),
        MType(type),
        Address(address),
        expiry(boost::posix_time::second_clock::local_time())
        {
          //we give it two heartbeat cycles of lifetime to start
          expiry = expiry + boost::posix_time::seconds(HEARTBEAT_INTERVAL_IN_SEC);
        }

        void refresh()
        {
          expiry = boost::posix_time::second_clock::local_time() +
            boost::posix_time::seconds(HEARTBEAT_INTERVAL_IN_SEC);
        }
    };

    //construct a predicate functor that is used to remove
    //expired workers
    struct expireFunctor
    {
     boost::posix_time::ptime heartbeat;
     expireFunctor(const boost::posix_time::ptime & t):
       heartbeat(t){}
     inline bool operator()(const WorkerInfo& worker) const
     {
       return worker.expiry < heartbeat;
     }
    };

    struct readyForWorkFunctor
    {
    readyForWorkFunctor(zmq::socketIdentity addr):
      Address(addr),
      Count(0)
      {
      }

    inline void operator()(WorkerInfo& worker)
      {
      if(worker.Address == this->Address)
        {
        worker.WaitingForWork = true;
        ++this->Count;
        }
      }
    zmq::socketIdentity Address;
    unsigned int Count; //number of elements we set to be ready for work
    };

    struct refreshWorkerFunctor
    {
    refreshWorkerFunctor(zmq::socketIdentity addr):
      Address(addr)
      {
      }

    inline void operator()(WorkerInfo& worker)
      {
      if(worker.Address == this->Address)
        {
        worker.refresh();
        }
      }
    zmq::socketIdentity Address;
    };

    typedef std::vector<WorkerInfo>::const_iterator ConstIt;
    typedef std::vector<WorkerInfo>::iterator It;
    std::vector<WorkerInfo> Pool;
};

//------------------------------------------------------------------------------
bool WorkerPool::addWorker(zmq::socketIdentity workerIdentity,
                           const remus::common::MeshIOType &type)
{
  this->Pool.push_back( WorkerPool::WorkerInfo(workerIdentity,type) );
  return true;
}

//------------------------------------------------------------------------------
bool WorkerPool::haveWaitingWorker(const remus::common::MeshIOType &type) const
{
  bool found = false;
  for(ConstIt i=this->Pool.begin(); !found && i != this->Pool.end(); ++i)
    {
    found = ( i->MType == type && i->WaitingForWork );
    }
  return found;
}

//------------------------------------------------------------------------------
bool WorkerPool::haveWorker(const zmq::socketIdentity& address) const
{
  bool found = false;
  for(ConstIt i=this->Pool.begin(); !found && i != this->Pool.end(); ++i)
    {
    found = ( i->Address == address );
    }
  return found;
}

//------------------------------------------------------------------------------
bool WorkerPool::readyForWork(const zmq::socketIdentity& address)
{
  //a worker can be registered multiple times, we need to iterate
  //over the entire vector and find all occurrences of the of the address
  WorkerPool::readyForWorkFunctor pred(address);

  //transform every element that matches the address to be waiting for work
  //need to use the resulting predicate to get the correct result.
  pred = std::for_each(this->Pool.begin(),this->Pool.end(), pred);
  return pred.Count > 0;
}


//------------------------------------------------------------------------------
zmq::socketIdentity WorkerPool::takeWorker(const remus::common::MeshIOType &type)
{

  bool found = false;
  int index = 0;
  WorkerInfo* info;
  for(index=0; index < this->Pool.size(); ++index)
    {
    info = &this->Pool[index];
    found = (info->MType == type) && (info->WaitingForWork);
    if(found)
      {
      break;
      }
    }

  if(found)
    {
    zmq::socketIdentity workerIdentity(info->Address);
    this->Pool.erase(this->Pool.begin()+index);
    return workerIdentity;
    }
  return zmq::socketIdentity();
}

//------------------------------------------------------------------------------
void WorkerPool::purgeDeadWorkers(const boost::posix_time::ptime& time)
{
  WorkerPool::expireFunctor pred(time);

  //remove if moves all bad items to end of the vector and returns
  //an iterator to the new end
  It newEnd = std::remove_if(this->Pool.begin(),this->Pool.end(),pred);

  //erase all the elements that remove_if moved to the end
  this->Pool.erase(newEnd,this->Pool.end());
}

//------------------------------------------------------------------------------
void WorkerPool::refreshWorker(const zmq::socketIdentity& address)
{
  std::for_each(this->Pool.begin(), this->Pool.end(),
              WorkerPool::refreshWorkerFunctor(address));
}
//------------------------------------------------------------------------------
std::set<zmq::socketIdentity> WorkerPool::livingWorkers() const
{
  std::set<zmq::socketIdentity> workerAddresses;
  for(ConstIt i=this->Pool.begin(); i != this->Pool.end(); ++i)
    {
    workerAddresses.insert(i->Address);
    }
  return workerAddresses;
}

}
}
}

#endif
