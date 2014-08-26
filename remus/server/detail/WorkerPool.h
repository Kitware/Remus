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

#include <remus/proto/JobRequirements.h>
#include <remus/proto/zmqSocketIdentity.h>

#include <remus/server/detail/SocketMonitor.h>

#include <set>
#include <vector>

namespace remus{
namespace server{
namespace detail{

class WorkerPool
{
public:
  WorkerPool();

  bool addWorker(zmq::SocketIdentity workerIdentity,
                 const remus::proto::JobRequirements& reqs);

  //return all the MeshIOTypes that workers have registered to support.
  //this allows the client to discover workers that have connected with
  //new types
  remus::common::MeshIOTypeSet supportedIOTypes() const;

  //return all the requirements for workers that are waiting.
  remus::proto::JobRequirementsSet
  waitingWorkerRequirements(remus::common::MeshIOType type) const;

  //do we have any worker waiting to take this type of job
  bool haveWaitingWorker(const remus::proto::JobRequirements& reqs) const;

  //do we have a worker with this address?
  bool haveWorker(const zmq::SocketIdentity& address,
                  const remus::proto::JobRequirements& reqs) const;

  //mark a worker with the given address ready to take a job.
  //returns false if a worker with that address wasn't found
  bool readyForWork(const zmq::SocketIdentity& address,
                    const remus::proto::JobRequirements& reqs);

  //returns the worker address and marks that the worker has taken a job
  zmq::SocketIdentity takeWorker(const remus::proto::JobRequirements& reqs);

  //remove all workers that haven't responded based on the passed in monitor
  void purgeDeadWorkers(remus::server::detail::SocketMonitor monitor);

  //return the socket identity of all workers
  std::set<zmq::SocketIdentity> allWorkers() const;

  //return the socket identity of all workers that want to work on a job
  std::set<zmq::SocketIdentity> allWorkersWantingWork() const;

private:
  struct WorkerInfo
  {
    int NumberOfDesiredJobs;
    remus::proto::JobRequirements Reqs;
    zmq::SocketIdentity Address;
    bool IsAlive; //alive as heartbeating, not alive as actively wanting jobs

    WorkerInfo(const zmq::SocketIdentity& address,
               const remus::proto::JobRequirements& type);

    bool isWaitingForWork() const { return NumberOfDesiredJobs > 0 && IsAlive; }
    void addJob() { ++NumberOfDesiredJobs; }
    void takesJob() { --NumberOfDesiredJobs; }
  };

  struct DeadWorkers
  {
   remus::server::detail::SocketMonitor Monitor;
   DeadWorkers(remus::server::detail::SocketMonitor monitor):
     Monitor(monitor){}
   inline bool operator()(const WorkerPool::WorkerInfo& worker)
    { return Monitor.isDead(worker.Address); }
  };


  typedef std::vector<WorkerInfo>::const_iterator ConstIt;
  typedef std::vector<WorkerInfo>::iterator It;
  std::vector<WorkerInfo> Pool;
};

}
}
}

#endif
