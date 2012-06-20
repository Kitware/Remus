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

#include <remus/JobResult.h>
#include <remus/JobStatus.h>
#include <remus/server/internal/uuidHelper.h>
#include <remus/common/zmqHelper.h>

#include <string>
#include <vector>

namespace remus{
namespace server{
namespace internal{

class WorkerPool
{
  public:
    WorkerPool():Pool(){}

    bool addWorker(zmq::socketIdentity workerIdentity,
                   const remus::MESH_TYPE& type);

    //do we have any worker waiting to take this type of job
    bool haveWaitingWorker(const remus::MESH_TYPE& type) const;

    //do we have a worker with this address?
    bool haveWorker(const zmq::socketIdentity& address) const;

    //mark a worker with the given adress ready to take a job.
    //returns false if a worker with that address wasn't found
    bool readyForWork(const zmq::socketIdentity& address);

    //returns the worker address and removes the worker from the pool
    zmq::socketIdentity takeWorker(const remus::MESH_TYPE& type);

    void purgeDeadWorkers(const boost::posix_time::ptime& time);

    void refreshWorker(const zmq::socketIdentity& address);

private:
    struct WorkerInfo
    {
      bool WaitingForWork;
      remus::MESH_TYPE MType;
      zmq::socketIdentity Address;
      boost::posix_time::ptime expiry; //after this time the job should be purged

      WorkerInfo(const zmq::socketIdentity& address, const remus::MESH_TYPE type):
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
     bool operator()(const WorkerInfo& worker)
     {
       return worker.expiry < heartbeat;
     }
    };

    typedef std::vector<WorkerInfo>::const_iterator ConstIt;
    typedef std::vector<WorkerInfo>::iterator It;
    std::vector<WorkerInfo> Pool;
};

//------------------------------------------------------------------------------
bool WorkerPool::addWorker(zmq::socketIdentity workerIdentity,
                           const remus::MESH_TYPE &type)
{
  this->Pool.push_back( WorkerPool::WorkerInfo(workerIdentity,type) );
  return true;
}

//------------------------------------------------------------------------------
bool WorkerPool::haveWaitingWorker(const MESH_TYPE &type) const
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
  bool found = false;
  for(It i=this->Pool.begin(); !found && i != this->Pool.end(); ++i)
    {
    if(i->Address == address)
      {
      found = true;
      i->WaitingForWork = true;
      }
    }
  return found;
}


//------------------------------------------------------------------------------
zmq::socketIdentity WorkerPool::takeWorker(const MESH_TYPE &type)
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
  // if(std::distance(newEnd,this->Pool.end()) > 0)
  //   {
  //   std::cout << "Purging dead workers, num " << std::distance(newEnd,this->Pool.end()) << std::endl;
  //   }

  //erase all the elements that remove_if moved to the end
  this->Pool.erase(newEnd,this->Pool.end());
}

//------------------------------------------------------------------------------
void WorkerPool::refreshWorker(const zmq::socketIdentity& address)
{
  for(It i=this->Pool.begin(); i != this->Pool.end(); ++i)
    {
    if( i->Address == address)
      {
      i->refresh();
      }
    }
}

}
}
}

#endif
