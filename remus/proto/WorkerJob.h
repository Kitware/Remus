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

#ifndef remus_proto_WorkerJob_h
#define remus_proto_WorkerJob_h

#include <string>

#include <remus/common/MeshIOType.h>
#include <remus/proto/JobSubmission.h>

//included for export symbols
#include <remus/proto/ProtoExports.h>

//suppress warnings inside boost headers for gcc and clang
REMUS_THIRDPARTY_PRE_INCLUDE
#include <boost/uuid/uuid.hpp>
REMUS_THIRDPARTY_POST_INCLUDE

// The remus::proto::WorkerJob class.
// For the server/worker the Job object represents all the information required
// to start an actual mesh job. Like the client side job object the Id and Type
// are  filled, but now we also have a JobSubmission object which contain the
// client side submission information. The worker needs the Id and Type
// information so that it can properly report back status and
// results to the server
namespace remus{
namespace proto{
class REMUSPROTO_EXPORT WorkerJob
{
public:
  enum JobValidity{ INVALID = 0, VALID_JOB, TERMINATE_WORKER};

  //construct an invalid job object
  WorkerJob();

  //construct a valid server side job object with Id, Type, and Submission
  WorkerJob(const boost::uuids::uuid& jid,
            const remus::proto::JobSubmission& sub);

  //get if the current job is a valid job
  bool valid() const { return Validity == VALID_JOB &&
                       this->type().valid(); }

  //Get the status of the message
  JobValidity validityReason() const { return Validity; }
  void updateValidityReason(JobValidity v){ Validity = v; }

  //get the id of the job
  const boost::uuids::uuid& id() const { return Id; }

  //get the mesh type of the job
  const remus::common::MeshIOType& type() const
    { return this->Submission.type(); }

  //get the submission object that was sent on the client
  const remus::proto::JobSubmission& submission() const
    { return Submission; }

  //helper method to easily get the contents of a given key
  //inside the submission as a std::string.
  std::string details(const std::string key) const;

  //helper method to easily extract a JobContent from the job submission
  bool details(const std::string& key, remus::proto::JobContent& value);

private:
  boost::uuids::uuid Id;
  remus::proto::JobSubmission Submission;

  JobValidity Validity;
};

//------------------------------------------------------------------------------
REMUSPROTO_EXPORT std::string to_string(const remus::proto::WorkerJob& job);


//------------------------------------------------------------------------------
REMUSPROTO_EXPORT remus::proto::WorkerJob to_WorkerJob(const std::string& msg);

}
}

#endif
