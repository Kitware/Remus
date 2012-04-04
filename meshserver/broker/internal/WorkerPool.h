/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __meshserver_broker_internal_ActiveJobState_h
#define __meshserver_broker_internal_ActiveJobState_h

#include <boost/date_time/posix_time/posix_time.hpp>

#include <meshserver/broker/internal/uuidHelper.h>
#include <meshserver/common/JobResult.h>
#include <meshserver/common/JobStatus.h>

#include <string>
#include <vector>

namespace meshserver{
namespace broker{
namespace internal{

class WorkerPool
{
  public:

    bool addWorker(const std::string& workerAddress,
                   const meshserver::MESH_TYPE& type);

    //do we have any worker waiting to take this type of job
    bool haveWaitingWorker(const meshserver::MESH_TYPE& type) const;

    //do we have a worker with this address?
    bool haveWorker(const std::string& address) const;

    //mark a worker with the given adress ready to take a job.
    //returns false if a worker with that address wasn't found
    bool readyForWork(const std::string& address);

    //returns the worker address and removes the worker from the pool
    std::string takeWorker(const meshserver::MESH_TYPE& type);

    void purgeDeadWorkers(const boost::posix_time::ptime& time);

    void refreshWorker(const std::string& address);

private:
    struct WorkerInfo
    {
      bool WaitingForWork;
      meshserver::MESH_TYPE MType;
      std::string Address;
      boost::posix_time::ptime expiry; //after this time the job should be purged

      WorkerInfo(const std::string& address, const meshserver::MESH_TYPE type):
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
bool WorkerPool::addWorker(const std::string &workerAddress,
                           const meshserver::MESH_TYPE &type)
{
  this->Pool.push_back( WorkerPool::WorkerInfo(workerAddress,type) );
  return true;
}

//------------------------------------------------------------------------------
bool WorkerPool::haveWaitingWorker(const MESH_TYPE &type) const
{
  bool found = false;
  for(ConstIt i=this->Pool.begin(); !found && i != this->Pool.end(); ++i)
    {
    found = ( (*i).MType == type && (*i).WaitingForWork );
    }
  return found;
}

//------------------------------------------------------------------------------
bool WorkerPool::haveWorker(const std::string& address) const
{
  bool found = false;
  for(ConstIt i=this->Pool.begin(); !found && i != this->Pool.end(); ++i)
    {
    found = ( (*i).Address == address );
    }
  return found;
}

//------------------------------------------------------------------------------
bool WorkerPool::readyForWork(const std::string& address)
{
  bool found = false;
  for(It i=this->Pool.begin(); !found && i != this->Pool.end(); ++i)
    {
    if((*i).Address == address)
      {
      found = true;
      (*i).WaitingForWork = true;
      }
    }
  return found;
}


//------------------------------------------------------------------------------
std::string WorkerPool::takeWorker(const MESH_TYPE &type)
{

  bool found = false;
  It i;
  for(i=this->Pool.begin(); !found && i != this->Pool.end(); ++i)
    {
    found = ( (*i).MType == type);
    }

  if(found)
    {
    std::string workerAddress = (*i).Address;
    this->Pool.erase(i);
    return workerAddress;
    }
  return std::string();
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
void WorkerPool::refreshWorker(const std::string& address)
{
  for(It i=this->Pool.begin(); i != this->Pool.end(); ++i)
    {
    if( (*i).Address == address)
      {
      (*i).refresh();
      }
    }
}

}
}
}

#endif
