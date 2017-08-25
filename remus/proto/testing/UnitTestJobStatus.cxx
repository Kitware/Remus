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

#include <remus/common/CompilerInformation.h>

REMUS_THIRDPARTY_PRE_INCLUDE
#include <boost/uuid/uuid.hpp>
REMUS_THIRDPARTY_POST_INCLUDE

#include <remus/proto/JobStatus.h>
#include <remus/testing/Testing.h>

namespace
{
using namespace remus::proto;

boost::uuids::uuid make_id()
{
  return remus::testing::UUIDGenerator();
}

void state_test()
{
  //try out all permutations of constructor
  JobStatus a(make_id(),remus::INVALID_STATUS);
  REMUS_ASSERT(a.failed());
  REMUS_ASSERT(!a.good());
  REMUS_ASSERT(!a.queued());
  REMUS_ASSERT(!a.inProgress());
  REMUS_ASSERT(!a.finished());

  JobStatus b(make_id(),remus::QUEUED);
  REMUS_ASSERT(!b.failed());
  REMUS_ASSERT(b.good());
  REMUS_ASSERT(b.queued());
  REMUS_ASSERT(!b.inProgress());
  REMUS_ASSERT(!b.finished());

  JobStatus c(make_id(),remus::IN_PROGRESS);
  REMUS_ASSERT(!c.failed());
  REMUS_ASSERT(c.good());
  REMUS_ASSERT(!c.queued());
  REMUS_ASSERT(c.inProgress());
  REMUS_ASSERT(!c.finished());

  JobStatus d(make_id(),remus::FINISHED);
  REMUS_ASSERT(!d.failed());
  REMUS_ASSERT(!d.good());
  REMUS_ASSERT(!d.queued());
  REMUS_ASSERT(!d.inProgress());
  REMUS_ASSERT(d.finished());

  JobStatus e(make_id(),remus::FAILED);
  REMUS_ASSERT(e.failed());
  REMUS_ASSERT(!e.good());
  REMUS_ASSERT(!e.queued());
  REMUS_ASSERT(!e.inProgress());
  REMUS_ASSERT(!e.finished());

  JobStatus f(make_id(),remus::EXPIRED);
  REMUS_ASSERT(f.failed());
  REMUS_ASSERT(!f.good());
  REMUS_ASSERT(!f.queued());
  REMUS_ASSERT(!f.inProgress());
  REMUS_ASSERT(!f.finished());

  JobStatus g(make_id(),JobProgress());
  REMUS_ASSERT(!g.failed());
  REMUS_ASSERT(g.good());
  REMUS_ASSERT(!g.queued());
  REMUS_ASSERT(g.inProgress());
  REMUS_ASSERT(!g.finished());

  JobProgress msg_progress("message"); //progress with just a message
  JobStatus h(make_id(),msg_progress);
  REMUS_ASSERT(!h.failed());
  REMUS_ASSERT(h.good());
  REMUS_ASSERT(!h.queued());
  REMUS_ASSERT(h.inProgress());
  REMUS_ASSERT(!h.finished());

  JobProgress value_progress(50); //progress with a value
  JobStatus i(make_id(),value_progress);
  REMUS_ASSERT(!i.failed());
  REMUS_ASSERT(i.good());
  REMUS_ASSERT(!i.queued());
  REMUS_ASSERT(i.inProgress());
  REMUS_ASSERT(!i.finished());

  //copy constructor
  JobStatus aa(a);
  REMUS_ASSERT(aa.failed());
  REMUS_ASSERT(!aa.good());
  REMUS_ASSERT(!aa.queued());
  REMUS_ASSERT(!aa.inProgress());
  REMUS_ASSERT(!aa.finished());
  REMUS_ASSERT( (aa==a) );
  REMUS_ASSERT( (!(a!=a)) );

  //copy assignment
  JobStatus bb = b;
  REMUS_ASSERT(!bb.failed());
  REMUS_ASSERT(bb.good());
  REMUS_ASSERT(bb.queued());
  REMUS_ASSERT(!bb.inProgress());
  REMUS_ASSERT(!bb.finished());
  REMUS_ASSERT( (bb==b) );
  REMUS_ASSERT( (!(b!=bb)) );

  //reuse ids
  JobStatus cc(c.id(), c.status());
  REMUS_ASSERT( (cc.id() == c.id()) );
  REMUS_ASSERT( (cc.status() == c.status()) );
  REMUS_ASSERT(!cc.failed());
  REMUS_ASSERT(cc.good());
  REMUS_ASSERT(!cc.queued());
  REMUS_ASSERT(cc.inProgress());
  REMUS_ASSERT(!cc.finished());
  REMUS_ASSERT( (cc==c) );
  REMUS_ASSERT( (!(c!=cc)) );

  //make sure 2 not equal jobStatus fail to
  //compare
  REMUS_ASSERT( !(cc==bb) );

  //verify that ccc different progress
  //value makes it non comparable
  JobStatus ccc(c.id(), value_progress);
  REMUS_ASSERT( (ccc!=c) );

}

void mark_test()
{
  JobStatus j(make_id(),JobProgress());
  REMUS_ASSERT(!j.failed());
  REMUS_ASSERT(j.good());
  REMUS_ASSERT(!j.queued());
  REMUS_ASSERT(j.inProgress());
  REMUS_ASSERT(!j.finished());

  j.markAsFinished();
  REMUS_ASSERT(!j.failed());
  REMUS_ASSERT(!j.good());
  REMUS_ASSERT(!j.queued());
  REMUS_ASSERT(!j.inProgress());
  REMUS_ASSERT(j.finished());

  j.markAsFailed();
  REMUS_ASSERT(j.failed());
  REMUS_ASSERT(!j.good());
  REMUS_ASSERT(!j.queued());
  REMUS_ASSERT(!j.inProgress());
  REMUS_ASSERT(!j.finished());
}

void progress_test()
{
  //really simple progress test
  JobProgress value_progress(50); //progress with a value
  JobStatus a(make_id(),value_progress);
  JobStatus b(make_id(),value_progress);

  REMUS_ASSERT( (a.progress() == b.progress()) );
  REMUS_ASSERT( (a.progress() == value_progress) );
  REMUS_ASSERT( (a.progress() == JobProgress(50)) );

  JobStatus c(make_id(), JobProgress());
  REMUS_ASSERT( (c.progress() != b.progress()) );
  REMUS_ASSERT( (c.progress() != value_progress) );
  REMUS_ASSERT( (c.progress() == JobProgress()) );

  c.updateProgress(value_progress);
  REMUS_ASSERT( (c.progress() == b.progress()) );
  REMUS_ASSERT( (c.progress() == value_progress) );
  REMUS_ASSERT( (c.progress() == JobProgress(50)) );

}

void validate_serialization(JobStatus s)
{
  std::string temp = to_string(s);
  JobStatus from_string = to_JobStatus(temp);

  REMUS_ASSERT( (from_string.id() == s.id()) );
  REMUS_ASSERT( (from_string.status() == s.status()) );
  REMUS_ASSERT( (from_string.progress() == s.progress()) );

  REMUS_ASSERT( (from_string.failed() == s.failed() ) );
  REMUS_ASSERT( (from_string.good() == s.good() ) );
  REMUS_ASSERT( (from_string.queued() == s.queued() ) );
  REMUS_ASSERT( (from_string.inProgress() == s.inProgress() ) );
  REMUS_ASSERT( (from_string.finished() == s.finished() ) );

}

void serialize_test()
{
  JobStatus a(make_id(),remus::EXPIRED);
  validate_serialization(a);

  JobStatus b(make_id(),remus::IN_PROGRESS);
  validate_serialization(b);

  JobProgress msg_progress("message"); //progress with just a message
  JobStatus c(make_id(),msg_progress);
  validate_serialization(c);


  JobProgress value_progress(50); //progress with a value
  JobStatus d(make_id(),value_progress);
  validate_serialization(d);

  JobProgress value_msg_progress(24,"message"); //progress with just a message
  JobStatus e(make_id(),value_msg_progress);
  validate_serialization(e);
}

void valid_test()
{
  JobStatus a(make_id(), remus::INVALID_STATUS);
  REMUS_ASSERT( (a.valid() == false) );
  REMUS_ASSERT( (a.invalid() == true) );

  JobStatus b(make_id(), remus::QUEUED);
  REMUS_ASSERT( (b.valid() == true) );
  REMUS_ASSERT( (b.invalid() == false) );

  JobStatus c(make_id(), remus::IN_PROGRESS);
  REMUS_ASSERT( (c.valid() == true) );
  REMUS_ASSERT( (c.invalid() == false) );

  JobStatus d(make_id(), remus::FINISHED);
  REMUS_ASSERT( (d.valid() == true) );
  REMUS_ASSERT( (d.invalid() == false) );

  JobStatus e(make_id(), remus::FAILED);
  REMUS_ASSERT( (e.valid() == true) );
  REMUS_ASSERT( (e.invalid() == false) );

  JobStatus f(make_id(), remus::EXPIRED);
  REMUS_ASSERT( (f.valid() == true) );
  REMUS_ASSERT( (f.invalid() == false) );
}

void make_functions()
{
  JobStatus a = make_JobStatus(make_id(), -25);
  REMUS_ASSERT( (a.progress().value() == 0) );
  REMUS_ASSERT( (a.inProgress() == true) );

  JobStatus b = make_JobStatus(make_id(), 25);
  REMUS_ASSERT( (b.progress().value() == 25) );
  REMUS_ASSERT( (b.inProgress() == true) );

  JobStatus c = make_JobStatus(make_id(), 100);
  REMUS_ASSERT( (c.progress().value() == 100) );
  REMUS_ASSERT( (c.inProgress() == true) );

  JobStatus d = make_JobStatus(make_id(), 125);
  REMUS_ASSERT( (d.progress().value() == 100) );
  REMUS_ASSERT( (d.inProgress() == true) );


  JobStatus e = make_JobStatus(make_id(), "message");
  REMUS_ASSERT( (e.progress().message() == "message") );
  REMUS_ASSERT( (e.inProgress() == true) );

  JobStatus f= make_JobStatus(make_id(), "");
  REMUS_ASSERT( (f.progress().message() == "") );
  REMUS_ASSERT( (f.inProgress() == true) );
  REMUS_ASSERT( (f.failed() == false) );

  JobStatus g= make_FailedJobStatus(make_id(), "error");
  REMUS_ASSERT( (g.progress().message() == "error") );
  REMUS_ASSERT( (g.inProgress() == false) );
  REMUS_ASSERT( (g.failed() == true) );

  JobStatus h= make_FailedJobStatus(make_id(), "");
  REMUS_ASSERT( (h.progress().message() == "") );
  REMUS_ASSERT( (h.failed() == true) );

}

}

int UnitTestJobStatus(int, char *[])
{
  state_test();
  mark_test();
  progress_test();
  serialize_test();
  valid_test();
  make_functions();

  return 0;
}
