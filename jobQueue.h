/*=========================================================================
  
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.
  
=========================================================================*/

#ifndef __jobQueue_h
#define __jobQueue_h

#include <boost/uuid/uuid.hpp>
#include <queue>

#include "Common/jobMessage.h"

namespace meshserver
{

struct WorkerMessage;

//lightweight struct that holds all jobs
struct QueuedJob
{
  boost::uuids::uuid Id;
  meshserver::WorkerMessage *message;
};

class JobQueue
{
public:
  JobQueue(){};


  bool push( const boost::uuids::uuid& id,
        meshserver::JobMessage* message)
  {

  }

  std::size_t size() { return Queue.size(); }

private:
  std::queue< meshserver::QueuedJob > Queue;
};


}

#endif // JOBQUEUE_H
