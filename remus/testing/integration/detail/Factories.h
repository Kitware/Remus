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
#ifndef remus_testing_integration_detail_Factories_h
#define remus_testing_integration_detail_Factories_h

#include <remus/server/WorkerFactoryBase.h>
#include <remus/worker/Worker.h>

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/thread/locks.hpp>

#include <remus/testing/integration/detail/Workers.h>

namespace remus {
namespace testing {
namespace integration {
namespace detail {

  //we want a custom factory that can not create any workers
  //but alays states that it can support a mesh type
  class AlwaysSupportFactory: public remus::server::WorkerFactoryBase
  {
  public:

    AlwaysSupportFactory( std::string workerName ):
      WorkerName(workerName)
    {
    }

    //when asked about which IOTypes we support we say none
    remus::common::MeshIOTypeSet supportedIOTypes() const
    {
      //we need to return that we support all types!
      return remus::common::generateAllIOTypes();
    }

    remus::proto::JobRequirementsSet workerRequirements(
                                            remus::common::MeshIOType type) const
    {
      //make clients think we have real workers, by sending back fake job reqs
      remus::proto::JobRequirements reqs =
           remus::proto::make_JobRequirements(type,WorkerName,"");
      remus::proto::JobRequirementsSet reqSet;
      reqSet.insert(reqs);
      return reqSet;
    }

    bool haveSupport(const remus::proto::JobRequirements& reqs) const
      {
      (void) reqs;
      //we want to return true so that the server adds jobs to it worker queue
      return true;
      }

    bool createWorker(const remus::proto::JobRequirements& type,
                      WorkerFactoryBase::FactoryDeletionBehavior lifespan)
      {
      (void) type;
      (void) lifespan;
      //we want to return false here so that server never thinks we are creating
      //a worker and assigns a job to a worker we didn't create
      return false;
      }

    void updateWorkerCount(){}
    unsigned int currentWorkerCount() const { return 0; }

    std::string WorkerName;
    };


//worker poll based server factory.
//You pass to the factory the requirements of the worker, the max number of
// workers you want.
class ThreadPoolWorkerFactory: public remus::server::WorkerFactoryBase
{
public:
  ThreadPoolWorkerFactory( remus::proto::JobRequirements reqs,
                           std::size_t maxThreadCount ):
    WorkerFactoryBase(),
    WorkerReqs(reqs),
    IOService(),
    Work( IOService ),
    ThreadPool(),
    Mutex(),
    AvailableThreads( maxThreadCount )
  {
    this->setMaxWorkerCount(maxThreadCount);

    for ( std::size_t i = 0; i < maxThreadCount; ++i )
    {
      this->ThreadPool.create_thread(
              boost::bind( &boost::asio::io_service::run,
                           &this->IOService ) );
    }
  }

  remus::proto::JobRequirementsSet workerRequirements(remus::common::MeshIOType type) const
  {
    remus::proto::JobRequirementsSet result;
    if(type == this->WorkerReqs.meshTypes())
      {
      result.insert( this->WorkerReqs );
      }
    return result;
  }

  remus::common::MeshIOTypeSet supportedIOTypes() const
  {
    remus::common::MeshIOTypeSet result;
    result.insert(this->WorkerReqs.meshTypes());
    return result;
  }

  bool haveSupport(const remus::proto::JobRequirements& reqs) const
  {
    return reqs == this->WorkerReqs;
  }

  bool createWorker(const remus::proto::JobRequirements& reqs,
                    remus::server::WorkerFactoryBase::FactoryDeletionBehavior /*lifespan*/)
  {
    if(this->currentWorkerCount() >= this->maxWorkerCount())
      {
      return false;
      }

    if( !(reqs == this->WorkerReqs) )
      {
      return false;
      }

    //mark a worker as being used
    {
    boost::lock_guard< boost::mutex > lock( this->Mutex );
    --this->AvailableThreads;
    }

    IOService.post( boost::bind( &ThreadPoolWorkerFactory::LaunchWorker, this ) );
    return true;
  }

  void updateWorkerCount()
  {
    //no need to do anything here the
  }

  unsigned int currentWorkerCount() const
  {
    boost::lock_guard< boost::mutex > lock( this->Mutex );
    return this->maxWorkerCount() - this->AvailableThreads;
  }

private:
  void LaunchWorker()
    {
    using namespace remus::testing;
      try
      {
      using namespace remus::worker;
      ServerConnection conn = make_ServerConnection(this->workerEndpoint());

      //construct the worker and tell it to process a job
      integration::detail::SingleShotWorker worker(this->WorkerReqs, conn);

      //should be 1, could be zero
      std::size_t numCompleted = worker.numberOfCompletedJobs();
      (void) numCompleted;
      }
      catch(...)
      { //the worker crashed, just ignore it
      }

    //worker is finished so mark the thread as usable
    {
    boost::lock_guard< boost::mutex > lock( this->Mutex );
    ++this->AvailableThreads;
    }

    }

  remus::proto::JobRequirements WorkerReqs;

  boost::asio::io_service IOService;
  boost::asio::io_service::work Work;
  boost::thread_group ThreadPool;

  mutable boost::mutex Mutex;
  std::size_t AvailableThreads;
};

}
}
}
}
#endif //remus_testing_integration_detail_Factories_h
