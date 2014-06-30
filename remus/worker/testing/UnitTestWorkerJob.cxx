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

#include <remus/worker/Job.h>
#include <remus/testing/Testing.h>

#include <string>

namespace {

using namespace remus::worker;

boost::uuids::uuid make_id()
{
  return remus::testing::UUIDGenerator();
}

remus::common::MeshIOType fakeMeshType()
{
  using namespace remus::meshtypes;
  int in_type = 1;
  int out_type = 3;
  return remus::common::MeshIOType(to_meshType(in_type),to_meshType(out_type));
}

remus::proto::JobSubmission make_empty_sub()
{
  using namespace remus::proto;

  //needs to be empty, but valid job, so we need a valid mesh type
  //on the requirements
  remus::common::MeshIOType mtype = fakeMeshType();
  JobRequirements r = make_JobRequirements(mtype," ", " ");
  return JobSubmission(r);
}

void verify_constructors()
{ //verify that all constructors including copy, and assignment work
  //and that the validity and job id doesn't change with any constructor

  Job invalid_job;
  Job valid_job(make_id(),make_empty_sub());

  REMUS_ASSERT( (invalid_job.valid() == false) );
  REMUS_ASSERT( (valid_job.valid() == true) );

  //verify the ids are different between a valid, and invalid job
  REMUS_ASSERT( (valid_job.id() != invalid_job.id()) );

  { //test copy and assignment with invalid job
  Job assignment_op = invalid_job;
  Job copy_constructor(invalid_job);
  REMUS_ASSERT( (assignment_op.valid() == false) );
  REMUS_ASSERT( (copy_constructor.valid() == false) );

  //verify the ids are the same
  REMUS_ASSERT( (assignment_op.id() == invalid_job.id()) );
  REMUS_ASSERT( (copy_constructor.id() == invalid_job.id()) );
  }

  { //test copy and assignment with valid job
  Job assignment_op = valid_job;
  Job copy_constructor(valid_job);
  REMUS_ASSERT( (assignment_op.valid() == true) );
  REMUS_ASSERT( (copy_constructor.valid() == true) );

  REMUS_ASSERT( (assignment_op.id() == valid_job.id()) );
  REMUS_ASSERT( (copy_constructor.id() == valid_job.id()) );
  }
}

void verify_validity()
{ //verify that we can change the validy of a job can be modified
  Job term_worker;

  term_worker.updateValidityReason(Job::TERMINATE_WORKER);
  REMUS_ASSERT( (term_worker.validityReason() == Job::TERMINATE_WORKER) );
  REMUS_ASSERT( (term_worker.valid() == false) );

  term_worker.updateValidityReason(Job::VALID_JOB);
  REMUS_ASSERT( (term_worker.validityReason() == Job::VALID_JOB) );
  //should still fail since it doesn't have an id
  REMUS_ASSERT( (term_worker.valid() == false) );


  Job valid_job(make_id(),make_empty_sub());
  REMUS_ASSERT( (valid_job.valid() == true) );

  valid_job.updateValidityReason(Job::INVALID);
  REMUS_ASSERT( (valid_job.validityReason() == Job::INVALID) );
  REMUS_ASSERT( (valid_job.valid() == false) );

  valid_job.updateValidityReason(Job::TERMINATE_WORKER);
  REMUS_ASSERT( (valid_job.validityReason() == Job::TERMINATE_WORKER) );
  REMUS_ASSERT( (valid_job.valid() == false) );

  valid_job.updateValidityReason(Job::VALID_JOB);
  REMUS_ASSERT( (valid_job.validityReason() == Job::VALID_JOB) );
  REMUS_ASSERT( (valid_job.valid() == true) );
}


void verify_meshTypes()
{ //verify that mesh types of a job stay consistent during copying

  Job invalid_job;

  remus::proto::JobSubmission sub = make_empty_sub();
  Job valid_job(make_id(),sub);

  REMUS_ASSERT( (!(invalid_job.type() == sub.type())) );
  REMUS_ASSERT( (valid_job.type() == sub.type()) );

  {
  Job assignment_op = valid_job;
  Job copy_constructor(valid_job);
  REMUS_ASSERT( (assignment_op.type() == valid_job.type()) );
  REMUS_ASSERT( (copy_constructor.type() == valid_job.type()) );
  }
}


void verify_submission()
{ //verify that the submission is correct, and that the details
  //method returns properly

  //test with an empty job sub
  remus::proto::JobSubmission empty_sub = make_empty_sub();
  Job empty_job(make_id(),empty_sub);

  REMUS_ASSERT( (empty_job.submission() == empty_sub) );
  REMUS_ASSERT( (empty_job.type() == empty_sub.type()) );

  {
  const std::string content =
                      empty_job.details(empty_job.submission().default_key());
  REMUS_ASSERT( (content.size() == 0) );
  }

  //now test with a job with a submission that has content
  remus::proto::JobSubmission sub_with_data = make_empty_sub();
  sub_with_data["non_default_key"] = remus::proto::make_JobContent("content");
  Job data_job(make_id(),sub_with_data);
  {
  const std::string content =
                      data_job.details(data_job.submission().default_key());
  REMUS_ASSERT( (content.size() == 0) );

  const std::string real_content = data_job.details("non_default_key");

  REMUS_ASSERT( (real_content.size() != 0) );
  REMUS_ASSERT( (real_content == "content") );
  }

}

} //namespace


int UnitTestWorkerJob(int, char *[])
{
  verify_constructors();
  verify_validity();
  verify_meshTypes();
  verify_submission();
  return 0;
}
