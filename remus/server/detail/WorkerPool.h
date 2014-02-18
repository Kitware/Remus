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

#ifndef remus_server_detail_WorkerPool_h
#define remus_server_detail_WorkerPool_h

#include <boost/date_time/posix_time/posix_time.hpp>

#include <remus/proto/JobRequirements.h>
#include <remus/proto/zmqHelper.h>

#include <set>
#include <vector>

namespace remus{
namespace server{
namespace detail{

class WorkerPool
{
public:
  WorkerPool();

  bool addWorker(zmq::socketIdentity workerIdentity,
                 const remus::proto::JobRequirements& reqs);

  //return all the requirements for workers that are waiting.
  remus::proto::JobRequirementsSet
  waitingWorkerRequirements(remus::common::MeshIOType type) const;

  //do we have any worker waiting to take this type of job
  bool haveWaitingWorker(const remus::proto::JobRequirements& reqs) const;

  //do we have a worker with this address?
  bool haveWorker(const zmq::socketIdentity& address) const;

  //mark a worker with the given address ready to take a job.
  //returns false if a worker with that address wasn't found
  bool readyForWork(const zmq::socketIdentity& address);

  //returns the worker address and removes the worker from the pool
  zmq::socketIdentity takeWorker(const remus::proto::JobRequirements& reqs);

  //remove all workers that haven't responded inside the heartbeat time
  void purgeDeadWorkers(const boost::posix_time::ptime& time);

  //keep all workers alive that responded inside the heartbeat time
  void refreshWorker(const zmq::socketIdentity& address);

  //return the socket identity of all living workers
  std::set<zmq::socketIdentity> livingWorkers() const;

private:
  struct WorkerInfo
  {
    bool WaitingForWork;
    remus::proto::JobRequirements Reqs;
    zmq::socketIdentity Address;
    //after this time the job should be purged
    boost::posix_time::ptime expiry;

    WorkerInfo(const zmq::socketIdentity& address,
               const remus::proto::JobRequirements& type);

    void refresh();
  };

  struct waitingTypes
  {
    void operator()( const WorkerInfo& winfo )
      { types.insert(winfo.Reqs); }
    remus::proto::JobRequirementsSet types;
  };

  struct ExpireWorkers
  {
   boost::posix_time::ptime heartbeat;
   ExpireWorkers(const boost::posix_time::ptime & t):
     heartbeat(t){}
   inline bool operator()(const WorkerPool::WorkerInfo& worker) const
    { return worker.expiry < heartbeat; }
  };

  struct ReadyForWork
  {
  ReadyForWork(zmq::socketIdentity addr):
    Address(addr),
    Count(0)
    { }

  inline void operator()(WorkerPool::WorkerInfo& worker)
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

  struct RefreshWorkers
  {
  RefreshWorkers(zmq::socketIdentity addr):
    Address(addr)
    { }

  inline void operator()(WorkerPool::WorkerInfo& worker)
    {
    if(worker.Address == this->Address)
      { worker.refresh(); }
    }
  zmq::socketIdentity Address;
  };

  typedef std::vector<WorkerInfo>::const_iterator ConstIt;
  typedef std::vector<WorkerInfo>::iterator It;
  std::vector<WorkerInfo> Pool;
};

}
}
}

#endif
