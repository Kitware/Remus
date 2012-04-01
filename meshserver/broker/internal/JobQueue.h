/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __meshserver_broker_internal_JobQueue_h
#define __meshserver_broker_internal_JobQueue_h

#include <boost/uuid/uuid.hpp>
#include <queue>
#include <set>

#include <meshserver/broker/internal/uuidHelper.h>
#include <meshserver/common/JobDetails.h>
#include <meshserver/JobMessage.h>

namespace meshserver{
namespace broker{
namespace internal{
//lightweight struct that holds all jobs
//Is a FIFO queue
class JobQueue
{
public:
  JobQueue():Queue(){}

  //Convert a JobMessage and UUID into a WorkerMessage.
  bool push( const boost::uuids::uuid& id,
             const meshserver::JobMessage& message);

  //Removes a job from the queue. And return it as
  //a JobDetail
  meshserver::common::JobDetails pop();

  //returns a reference to the mesh at the front
  const meshserver::JobMessage& front();

  //Returns true if we contain the UUID
  bool haveUUID(const boost::uuids::uuid& id) const;

  //return the number of jobs queued up
  std::size_t size() { return Queue.size(); }

private:
  struct QueuedJob
  {
    QueuedJob(const boost::uuids::uuid& id,
              const meshserver::JobMessage& message):
              Id(id),Message(message){}
    const boost::uuids::uuid Id;
    meshserver::JobMessage Message;
  };
  std::queue<QueuedJob> Queue;
  std::set<boost::uuids::uuid> QueuedIds;

  //make copying not possible
  JobQueue (const JobQueue&);
  void operator = (const JobQueue&);
};


//------------------------------------------------------------------------------
bool JobQueue::push(const boost::uuids::uuid &id,
                    const meshserver::JobMessage& message)
{
  this->Queue.push(QueuedJob(id,message));
  this->QueuedIds.insert(id);
}

//------------------------------------------------------------------------------
meshserver::common::JobDetails JobQueue::pop()
{
  QueuedJob job = this->Queue.front();

  //remove the job
  this->Queue.pop();
  this->QueuedIds.erase(job.Id);

  //convert the queued job into a job detail
  const std::string data(job.Message.data(),job.Message.dataSize());
  return meshserver::common::JobDetails(job.Id,data);
}

//------------------------------------------------------------------------------
const meshserver::JobMessage& JobQueue::front()
{
  return this->Queue.front().Message;
}

//------------------------------------------------------------------------------
bool JobQueue::haveUUID(const boost::uuids::uuid &id) const
{
  return this->QueuedIds.count(id) == 1 ? true : false;
}


}
}
} //namespace meshserver::broker::internal

#endif // JOBQUEUE_H
