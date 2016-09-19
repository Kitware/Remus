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

#include <remus/server/ThreadWorkerFactory.h>

#include <remus/common/CompilerInformation.h>
#include <remus/common/MeshIOType.h>
#include <remus/common/SleepFor.h>
#include <remus/proto/JobRequirements.h>

#include <algorithm>
#include <exception>
#include <functional>
#include <future>
#include <map>
#include <mutex>
#include <numeric>

namespace remus{
namespace server{

class ThreadWorkerFactory::ThreadPool
{
private:
  struct FutureWithState
  {
    FutureWithState& operator=(std::future<void>&& future)
    {
      this->Future = std::move(future);
      this->Completed = false;
      return *this;
    }
  
    std::future<void> Future;
    bool Completed;
  };

  std::map<std::size_t, FutureWithState > Futures;
  std::size_t PoolSize;
  std::mutex Mutex;

public:

  /// @brief Constructor.
  ThreadPool( std::size_t poolSize ) :
    PoolSize( poolSize )
  {
  }

  /// @brief Destructor.
  ~ThreadPool()
  {
    for ( auto& f : this->Futures )
      {
      f.second.Future.wait();
      }
  }

  /// @brief Change the maximum number of threads. If the input is smaller than
  ///        the number of currently active threads, the pool will wait for the
  ///        current threads to finish.
  void resize( std::size_t size )
  {
    this->PoolSize = size;
  }

  /// @brief Return the maximum number of active threads.
  std::size_t max_threads() const
  {
    return this->PoolSize;
  }

  /// @brief Return the current number of active threads.
  std::size_t number_of_active_threads() const
  {
    typedef std::pair< const std::size_t, FutureWithState > FutureElement;
    return std::accumulate( this->Futures.cbegin(),
			    this->Futures.cend(),
			    0,
			    []( int counter, const FutureElement& f )
			    { return counter + !f.second.Completed; } );
  }

  /// @brief Adds a task to the thread pool if a thread is currently available.
  template < typename Task >
  bool run_task( Task task )
  {
    std::unique_lock< std::mutex > lock( this->Mutex );

    // Harvest completed futures.
    auto it = this->Futures.begin(); 
    while ( it != this->Futures.end() )
      {
      if (it->second.Completed)
	{
	this->Futures.erase(it++);
	}
      else
	{
	++it;
	}
      }

    static std::size_t key = 0;

    // If no threads are available, then return.
    if ( this->Futures.size() >= PoolSize )
      {
      return false;
      }

    // add a wrapped task into the list.
    this->Futures[key] = std::async( std::launch::async,
                                     &ThreadPool::wrap_task,
                                     this,
                                     std::function< void() >( task ),
                                     key );
    ++key;
    return true;
  }

private:
  /// @brief Wrap a task so that the available count can be increased once
  ///        the user provided task has completed.
  void wrap_task( std::function< void() > task, std::size_t key )
  {
    // Run the user supplied task.
    try
    {
      task();
    }
    // Suppress all exceptions.
    catch ( const std::exception& ) {}

    // Task has finished, so we clean up.
    std::unique_lock< std::mutex > lock( this->Mutex );

    // the future for this task is marked as Completed. We cannot simply
    // delete it because that would force the task to return, which would
    // result in deadlock.
    this->Futures[ key ].Completed = true;
  }
};


//----------------------------------------------------------------------------
ThreadWorkerFactory::ThreadWorkerFactory():
  WorkerFactoryBase()
{
  this->Pool = new ThreadPool(1);
}

//----------------------------------------------------------------------------
ThreadWorkerFactory::~ThreadWorkerFactory()
{
  delete this->Pool;
}

//----------------------------------------------------------------------------
remus::common::MeshIOTypeSet ThreadWorkerFactory::supportedIOTypes() const
{
  // Go through the available worker thread functors and collect the supported
  // mesh types associated with each registered JobRequirements.
  return std::accumulate(
    this->WorkerThreadTypes.begin(), this->WorkerThreadTypes.end(),
    remus::common::MeshIOTypeSet(),
    [&]( remus::common::MeshIOTypeSet &s, const std::pair<JobRequirements,
         WorkerThread>& c) -> remus::common::MeshIOTypeSet&
    { s.insert(c.first.meshTypes()); return s; });
}

//----------------------------------------------------------------------------
remus::proto::JobRequirementsSet ThreadWorkerFactory::workerRequirements(
                                       remus::common::MeshIOType type) const
{
  // Go through the available worker thread functors and collect the supported
  // JobRequirements for each worker that supports <type>.
  return std::accumulate(
    this->WorkerThreadTypes.begin(), this->WorkerThreadTypes.end(),
    remus::proto::JobRequirementsSet(),
    [&]( remus::proto::JobRequirementsSet &s, const std::pair<JobRequirements,
         WorkerThread>& c) -> remus::proto::JobRequirementsSet&
    { if (c.first.meshTypes() == type) s.insert(c.first); return s; });
}

//----------------------------------------------------------------------------
bool ThreadWorkerFactory::haveSupport(
                            const remus::proto::JobRequirements& reqs) const
{
  // Check each work thread functor to see if it satisfies the requested
  // JobRequirements.
  for (auto& worker : this->WorkerThreadTypes)
    {
    if (reqs == worker.first)
      {
      return true;
      }
    }
  return false;
}

//----------------------------------------------------------------------------
bool ThreadWorkerFactory::registerWorkerType(
  const remus::proto::JobRequirements& requirements, WorkerThread worker)
{
  // Simply add the work thread functor to the map.
  this->WorkerThreadTypes[requirements] = worker;
  return true;
}

//----------------------------------------------------------------------------
bool ThreadWorkerFactory::createWorker(
  const remus::proto::JobRequirements& reqs,
  WorkerFactoryBase::FactoryDeletionBehavior lifespan)
{
  // Reject workers slated to live beyond the factory's lifespan, and process
  // the rest.
  if (lifespan == LiveOnFactoryDeletion)
    {
    return false;
    }

  return this->launchWorkerThread(reqs);
}

//----------------------------------------------------------------------------
bool ThreadWorkerFactory::launchWorkerThread(
  const remus::proto::JobRequirements& requirements)
{
  // Reject workers if the maximum worker count has been reached.
  if (this->currentWorkerCount() >= this->maxWorkerCount())
    {
    return false;
    }

  // Reject workers if the type is unsupported.
  auto search = this->WorkerThreadTypes.find(requirements);
  if(search == this->WorkerThreadTypes.end())
    {
    return false;
    }

  // Launch the worker.
  return this->addWorker( std::bind(search->second, requirements,
                                    this->workerEndpoint() ) );
}

//----------------------------------------------------------------------------
void ThreadWorkerFactory::setMaxWorkerCount(unsigned int count)
{
  this->Pool->resize(count);
}

//----------------------------------------------------------------------------
unsigned int ThreadWorkerFactory::maxWorkerCount() const
{
  return static_cast<unsigned int>(this->Pool->max_threads());
}

//----------------------------------------------------------------------------
unsigned int ThreadWorkerFactory::currentWorkerCount() const
{
  return static_cast<unsigned int>(this->Pool->number_of_active_threads());
}

//----------------------------------------------------------------------------
bool ThreadWorkerFactory::addWorker(std::function<void()> workerWithTask)
{
  return this->Pool->run_task(workerWithTask);
}


}
}
