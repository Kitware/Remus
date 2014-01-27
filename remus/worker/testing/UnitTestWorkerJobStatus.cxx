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

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/random_generator.hpp>
#include <remus/worker/JobStatus.h>
#include <remus/testing/Testing.h>

namespace
{
using namespace remus::worker;
boost::uuids::random_generator generator;

void state_test()
{
  //try out all permutations of constructor
  JobStatus a(generator(),remus::INVALID_STATUS);
  REMUS_ASSERT(a.failed());
  REMUS_ASSERT(!a.inProgress());
  REMUS_ASSERT(!a.finished());

  JobStatus b(generator(),remus::QUEUED);
  REMUS_ASSERT(!b.failed());
  REMUS_ASSERT(!b.inProgress());
  REMUS_ASSERT(!b.finished());

  JobStatus c(generator(),remus::IN_PROGRESS);
  REMUS_ASSERT(!c.failed());
  REMUS_ASSERT(c.inProgress());
  REMUS_ASSERT(!c.finished());

  JobStatus d(generator(),remus::FINISHED);
  REMUS_ASSERT(!d.failed());
  REMUS_ASSERT(!d.inProgress());
  REMUS_ASSERT(d.finished());

  JobStatus e(generator(),remus::FAILED);
  REMUS_ASSERT(e.failed());
  REMUS_ASSERT(!e.inProgress());
  REMUS_ASSERT(!e.finished());

  JobStatus f(generator(),remus::EXPIRED);
  REMUS_ASSERT(f.failed());
  REMUS_ASSERT(!f.inProgress());
  REMUS_ASSERT(!f.finished());

  JobStatus g(generator(),JobProgress());
  REMUS_ASSERT(!g.failed());
  REMUS_ASSERT(g.inProgress());
  REMUS_ASSERT(!g.finished());

  JobProgress msg_progress("message"); //progress with just a message
  JobStatus h(generator(),msg_progress);
  REMUS_ASSERT(!h.failed());
  REMUS_ASSERT(h.inProgress());
  REMUS_ASSERT(!h.finished());

  JobProgress value_progress(50); //progress with a value
  JobStatus i(generator(),value_progress);
  REMUS_ASSERT(!i.failed());
  REMUS_ASSERT(i.inProgress());
  REMUS_ASSERT(!i.finished());

  //copy constructor
  JobStatus aa(a);
  REMUS_ASSERT(aa.failed());
  REMUS_ASSERT(!aa.inProgress());
  REMUS_ASSERT(!aa.finished());

  //copy assignment
  JobStatus bb = b;
  REMUS_ASSERT(!bb.failed());
  REMUS_ASSERT(!bb.inProgress());
  REMUS_ASSERT(!bb.finished());

  //reuse ids
  JobStatus cc(c.JobId, c.Status);
  REMUS_ASSERT( (cc.JobId == c.JobId) );
  REMUS_ASSERT( (cc.Status == c.Status) );
  REMUS_ASSERT(!cc.failed());
  REMUS_ASSERT(cc.inProgress());
  REMUS_ASSERT(!cc.finished());


  //verify you can move a state to failed
  JobStatus in_prog(generator(),remus::IN_PROGRESS);
  REMUS_ASSERT(!in_prog.failed());
  REMUS_ASSERT(in_prog.inProgress());
  REMUS_ASSERT(!in_prog.finished());

  in_prog.hasFailed();
  REMUS_ASSERT(in_prog.failed());
  REMUS_ASSERT(!in_prog.inProgress());
  REMUS_ASSERT(!in_prog.finished());

  in_prog.hasFinished();
  REMUS_ASSERT(!in_prog.failed());
  REMUS_ASSERT(!in_prog.inProgress());
  REMUS_ASSERT(in_prog.finished());
}

void progress_test()
{
  //really simple progress test
  JobProgress value_progress(50); //progress with a value
  JobStatus a(generator(),value_progress);
  JobStatus b(generator(),value_progress);


  REMUS_ASSERT( (a.progress() == b.progress()) );
  REMUS_ASSERT( (a.progress() == value_progress) );
  REMUS_ASSERT( (a.progress() == JobProgress(50)) );

  //swap the progress object on an items
  JobStatus c(generator(),remus::QUEUED);
  c.updateProgress(value_progress);
  REMUS_ASSERT( (c.progress() == a.progress()) );
  REMUS_ASSERT(c.inProgress());
}

void validate_serialization(JobStatus s)
{
  std::string temp = to_string(s);
  JobStatus from_string = to_JobStatus(temp);

  REMUS_ASSERT( (from_string.JobId == s.JobId) );
  REMUS_ASSERT( (from_string.Status == s.Status) );
  REMUS_ASSERT( (from_string.progress() == s.progress()) );

  REMUS_ASSERT( (from_string.failed() == s.failed() ) );
  REMUS_ASSERT( (from_string.inProgress() == s.inProgress() ) );
  REMUS_ASSERT( (from_string.finished() == s.finished() ) );
}

void serialize_test()
{
  JobStatus a(generator(),remus::EXPIRED);
  validate_serialization(a);

  JobStatus b(generator(),remus::IN_PROGRESS);
  validate_serialization(b);

  JobProgress msg_progress("message"); //progress with just a message
  JobStatus c(generator(),msg_progress);
  validate_serialization(c);


  JobProgress value_progress(50); //progress with a value
  JobStatus d(generator(),value_progress);
  validate_serialization(d);

  JobProgress value_msg_progress("message"); //progress with just a message
  JobStatus e(generator(),msg_progress);
  validate_serialization(e);
}


}

int UnitTestWorkerJobStatus(int, char *[])
{
  state_test();
  progress_test();
  serialize_test();

  return 0;
}
