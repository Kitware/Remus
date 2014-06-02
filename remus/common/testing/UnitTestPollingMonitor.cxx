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

#include <boost/date_time/posix_time/posix_time.hpp>

#include <remus/common/PollingMonitor.h>

#include <remus/testing/Testing.h>

namespace
{
class TestPoller : public remus::common::PollingMonitor
{
  public:
    //setup the test poller with an initial time
    TestPoller(boost::posix_time::ptime* t):PollingMonitor()
    {
      //work around to clear the superclasses default value in the
      //monitoring
      boost::posix_time::ptime start = *t;
      start -= boost::posix_time::seconds(10 * this->minTimeOut());
      for(int i=0; i < 10; ++i)
        {
        start += boost::posix_time::seconds(this->minTimeOut());
        PollingMonitor::pollOccurredAt(&start);
        }
    }

    void pollOccurredAt( boost::posix_time::ptime* t )
      { PollingMonitor::pollOccurredAt(t); }

};

void verify_constructors()
{
  //just verify that we have a copy constructor
  remus::common::PollingMonitor p;
  p = remus::common::PollingMonitor();

  remus::common::PollingMonitor p2(p);
}

void verify_shared_ptr()
{
  remus::common::PollingMonitor p;
  remus::common::PollingMonitor p2(p);

  remus::common::PollingMonitor notP;

  //now modify p multiple times to fill the buffer
  p.pollOccurred();
  p.pollOccurred();
  p.pollOccurred();
  p.pollOccurred();
  p.pollOccurred();
  p.pollOccurred();

  //since we state a min cap for the poll rate, current can't be trusted to
  //be different, but average will be
  REMUS_ASSERT( (p2.average() == p.average()) );
  REMUS_ASSERT( (p2.average() != notP.average()) );

  REMUS_ASSERT( (p2.durationOfTheLastPoll() == p.durationOfTheLastPoll()) );
  REMUS_ASSERT( (p2.durationOfTheLastPoll() != notP.durationOfTheLastPoll()) );
}

void verify_min_max()
{
  //verify that even inverted min & max are properly handled
  remus::common::PollingMonitor p(10,240);
  remus::common::PollingMonitor p2(240,10);

  REMUS_ASSERT( (p.minTimeOut() == p2.minTimeOut()) );
  REMUS_ASSERT( (p.maxTimeOut() == p2.maxTimeOut()) );

  REMUS_ASSERT( (p.minTimeOut()  <= p.maxTimeOut()) );
  REMUS_ASSERT( (p2.minTimeOut() <= p2.maxTimeOut()) );

  remus::common::PollingMonitor defValues;
  REMUS_ASSERT( (defValues.minTimeOut() <= defValues.maxTimeOut()) );
}

void verify_fast_polling()
{
  boost::posix_time::ptime current =
                              boost::posix_time::microsec_clock::local_time();
  boost::posix_time::ptime t = current - boost::posix_time::hours(5);


  TestPoller p(&t);

  //verify polling at the min keeps the duration equal to the min
  const unsigned long minTime = p.minTimeOut();
  const unsigned long inputTime = (p.minTimeOut() - 1);
  for(int i=0; i < 20; ++i)
  {
  t += boost::posix_time::seconds(inputTime);
  p.pollOccurredAt(&t);

  //since we are polling faster than the min, we expect that the
  //current poll rate should be equal to the min
  REMUS_ASSERT ( (p.durationOfTheLastPoll(  ) == inputTime) );

  //we expect that the current will be the min
  REMUS_ASSERT ( (p.current( ) == minTime) );

  //we expect that the average will be less than the min and greater than
  //the average. Since the poller starts with the average equal to the min
  //the average won't always be equal to input time
  REMUS_ASSERT ((p.average(  ) < minTime && (p.average(  ) >= inputTime)) );
  }

  //by 20 iterations we should have an average that is equal to the inputTime
  REMUS_ASSERT ( (p.average(  ) == inputTime) );

  REMUS_ASSERT ( (p.hasAbnormalEvent( ) == false) );
}

void verify_slow_polling()
{
  boost::posix_time::ptime current =
                              boost::posix_time::microsec_clock::local_time();
  boost::posix_time::ptime t = current - boost::posix_time::hours(5);


  TestPoller p(&t);

  //verify polling at the min keeps the duration equal to the min
  const int pollTimeInSecs = p.minTimeOut() +
                                      (p.maxTimeOut() - p.minTimeOut())/2;
  for(int i=0; i < 20; ++i)
  {
  t += boost::posix_time::seconds(pollTimeInSecs);
  p.pollOccurredAt(&t);

  //we expect that the current will be always greater than min
  REMUS_ASSERT ( (p.current( ) > p.minTimeOut()) );
  REMUS_ASSERT ( (p.average( ) > p.minTimeOut()) );
  REMUS_ASSERT ( (p.average( ) <= pollTimeInSecs) );
  REMUS_ASSERT ( (p.hasAbnormalEvent( ) == false) );
  }

  REMUS_ASSERT ( (p.average( ) == pollTimeInSecs) );
  REMUS_ASSERT ( (p.hasAbnormalEvent( ) == false) );

  // verify that as we use p.current() for the duration,
  // we reach p.maxTimeOut as the current and average
  for(int i=0; i < 20; ++i)
  {
  boost::int64_t previous_current = p.current();
  boost::int64_t previous_average = p.average();
  t += boost::posix_time::seconds(static_cast<long>(p.current()));
  p.pollOccurredAt(&t);

  REMUS_ASSERT ( (p.current( ) >= previous_current ) );
  REMUS_ASSERT ( (p.average( ) >= previous_average ) );
  REMUS_ASSERT ( (p.average( ) <= p.maxTimeOut()) );
  REMUS_ASSERT ( (p.hasAbnormalEvent( ) == false) );
  }

  REMUS_ASSERT ( (p.average( ) == p.maxTimeOut()) );
  REMUS_ASSERT ( (p.current( ) == p.maxTimeOut()) );

  REMUS_ASSERT ( (p.hasAbnormalEvent( ) == false) );
}


void verify_handles_resumes()
{
  //verify if that we get an occurrence of a significant delta between
  //two polling events we don't give a bad value for current()
  boost::posix_time::ptime current =
                              boost::posix_time::microsec_clock::local_time();
  boost::posix_time::ptime t = current - boost::posix_time::hours(5);

  //start 5 hours before now
  TestPoller p(&t);

  t += boost::posix_time::hours(10);
  p.pollOccurredAt(&t);

  REMUS_ASSERT ( (p.current( ) == p.maxTimeOut()) );
  REMUS_ASSERT ( (p.average( ) > p.maxTimeOut()) );
  REMUS_ASSERT ( (p.hasAbnormalEvent( ) == true) );

  t += boost::posix_time::hours(10);
  p.pollOccurredAt(&t);

  REMUS_ASSERT ( (p.current( ) == p.maxTimeOut()) );
  REMUS_ASSERT ( (p.average( ) > p.maxTimeOut()) );
  REMUS_ASSERT ( (p.hasAbnormalEvent( ) == true) );

  while(p.current() == p.maxTimeOut())
    {
    t += boost::posix_time::seconds(10);
    p.pollOccurredAt(&t);
    }
  REMUS_ASSERT ( (p.average( ) < p.maxTimeOut()) );
  REMUS_ASSERT ( (p.hasAbnormalEvent( ) == false) );
}

}


int UnitTestPollingMonitor(int, char *[])
{
  verify_constructors();
  verify_shared_ptr();
  verify_min_max();
  verify_fast_polling();
  verify_slow_polling();

  verify_handles_resumes();

  return 0;
}
