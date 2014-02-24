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

#ifndef remus_server_detail_ActiveJobs_h
#define remus_server_detail_ActiveJobs_h

#include <remus/proto/JobResult.h>
#include <remus/proto/JobStatus.h>
#include <remus/proto/zmqHelper.h>

#include <boost/date_time/posix_time/posix_time.hpp>

#include <map>
#include <set>

namespace remus{
namespace server{
namespace detail{

class ActiveJobs
{
  public:
    ActiveJobs():Info(){}

    bool add(const zmq::socketIdentity& workerIdentity,
             const boost::uuids::uuid& id);

    bool remove(const boost::uuids::uuid& id);

    zmq::socketIdentity workerAddress(const boost::uuids::uuid& id) const;

    bool haveUUID(const boost::uuids::uuid& id) const;

    bool haveResult(const boost::uuids::uuid& id) const;

    //returns a worker side job status object for a job
    const remus::proto::JobStatus& status(const boost::uuids::uuid& id);

    //returns a worker side job result object for a job
    const remus::proto::JobResult& result(const boost::uuids::uuid& id);

    //update the job status of a job.
    //valid values are:
    // QUEUED
    // IN_PROGRESS
    // FAILED
    // EXPIRED
    // To update a job to the finished state, you have to call updateResult
    // not update status
    void updateStatus(const remus::proto::JobStatus& s);

    void updateResult(const remus::proto::JobResult& r);

    void markExpiredJobs(const boost::posix_time::ptime& time);

    void refreshJobs(const zmq::socketIdentity &workerIdentity);

    std::set<zmq::socketIdentity> activeWorkers() const;

private:
    struct JobState
    {
      zmq::socketIdentity WorkerAddress;
      remus::proto::JobStatus jstatus;
      remus::proto::JobResult jresult;
      boost::posix_time::ptime expiry; //after this time the job should be purged
      bool haveResult;

      JobState(const zmq::socketIdentity& workerIdentity,
               const boost::uuids::uuid& id,
               remus::STATUS_TYPE stat);

      void refresh();

      bool canUpdateStatusTo(remus::proto::JobStatus s) const;
    };

    typedef std::pair<boost::uuids::uuid, JobState> InfoPair;
    typedef std::map< boost::uuids::uuid, JobState>::const_iterator InfoConstIt;
    typedef std::map< boost::uuids::uuid, JobState>::iterator InfoIt;
    std::map<boost::uuids::uuid, JobState> Info;
};

}
}
}
#endif
