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
#include <remus/client/JobStatus.h>
#include <remus/testing/Testing.h>

namespace
{
using namespace remus::client;
boost::uuids::random_generator generator;

void state_test()
{
  //try out all permutations of constructor
  JobStatus a(generator(),remus::INVALID_STATUS);
  REMUS_ASSERT(a.failed());
  REMUS_ASSERT(!a.good());
  REMUS_ASSERT(!a.queued());
  REMUS_ASSERT(!a.inProgress());
  REMUS_ASSERT(!a.finished());

  JobStatus b(generator(),remus::QUEUED);
  REMUS_ASSERT(!b.failed());
  REMUS_ASSERT(b.good());
  REMUS_ASSERT(b.queued());
  REMUS_ASSERT(!b.inProgress());
  REMUS_ASSERT(!b.finished());

  JobStatus c(generator(),remus::IN_PROGRESS);
  REMUS_ASSERT(!c.failed());
  REMUS_ASSERT(c.good());
  REMUS_ASSERT(!c.queued());
  REMUS_ASSERT(c.inProgress());
  REMUS_ASSERT(!c.finished());

  JobStatus d(generator(),remus::FINISHED);
  REMUS_ASSERT(!d.failed());
  REMUS_ASSERT(!d.good());
  REMUS_ASSERT(!d.queued());
  REMUS_ASSERT(!d.inProgress());
  REMUS_ASSERT(d.finished());

  JobStatus e(generator(),remus::FAILED);
  REMUS_ASSERT(e.failed());
  REMUS_ASSERT(!e.good());
  REMUS_ASSERT(!e.queued());
  REMUS_ASSERT(!e.inProgress());
  REMUS_ASSERT(!e.finished());

  JobStatus f(generator(),remus::EXPIRED);
  REMUS_ASSERT(f.failed());
  REMUS_ASSERT(!f.good());
  REMUS_ASSERT(!f.queued());
  REMUS_ASSERT(!f.inProgress());
  REMUS_ASSERT(!f.finished());

  JobStatus g(generator(),JobProgress());
  REMUS_ASSERT(!g.failed());
  REMUS_ASSERT(g.good());
  REMUS_ASSERT(!g.queued());
  REMUS_ASSERT(g.inProgress());
  REMUS_ASSERT(!g.finished());

  JobProgress msg_progress("message"); //progress with just a message
  JobStatus h(generator(),msg_progress);
  REMUS_ASSERT(!h.failed());
  REMUS_ASSERT(h.good());
  REMUS_ASSERT(!h.queued());
  REMUS_ASSERT(h.inProgress());
  REMUS_ASSERT(!h.finished());

  JobProgress value_progress(50); //progress with a value
  JobStatus i(generator(),value_progress);
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

  //copy assignment
  JobStatus bb = b;
  REMUS_ASSERT(!bb.failed());
  REMUS_ASSERT(bb.good());
  REMUS_ASSERT(bb.queued());
  REMUS_ASSERT(!bb.inProgress());
  REMUS_ASSERT(!bb.finished());

  //reuse ids
  JobStatus cc(c.id(), c.status());
  REMUS_ASSERT( (cc.id() == c.id()) );
  REMUS_ASSERT( (cc.status() == c.status()) );
  REMUS_ASSERT(!cc.failed());
  REMUS_ASSERT(cc.good());
  REMUS_ASSERT(!cc.queued());
  REMUS_ASSERT(cc.inProgress());
  REMUS_ASSERT(!cc.finished());

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

int UnitTestClientJobStatus(int, char *[])
{
  state_test();
  progress_test();
  serialize_test();

  return 0;
}
