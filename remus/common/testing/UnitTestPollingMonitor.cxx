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

#ifndef _MSC_VER
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
#include <boost/date_time/posix_time/posix_time.hpp>
#ifndef _MSC_VER
  #pragma GCC diagnostic pop
#endif

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
      start -= boost::posix_time::milliseconds( 10 * this->minTimeOut() );
      for(int i=0; i < 10; ++i)
        {
        start += boost::posix_time::milliseconds( this->minTimeOut() );
        PollingMonitor::fakeAPollOccurringAt(&start);
        }
    }
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

  //verify that negative value are handled properly
  remus::common::PollingMonitor negative_v(-10,-240);
  REMUS_ASSERT( (negative_v.minTimeOut() == boost::int64_t(0)) );
  REMUS_ASSERT( (negative_v.maxTimeOut() == boost::int64_t(0)) );

  remus::common::PollingMonitor negative_low(-10,240);
  REMUS_ASSERT( (negative_low.minTimeOut() == boost::int64_t(0)) );
  REMUS_ASSERT( (negative_low.maxTimeOut() == boost::int64_t(240)) );

  //this is the funky one, since -240 becomes 0, the range is 0,10
  remus::common::PollingMonitor negative_high(10,-240);
  REMUS_ASSERT( (negative_high.minTimeOut() == boost::int64_t(0)) );
  REMUS_ASSERT( (negative_high.maxTimeOut() == boost::int64_t(10)) );

  remus::common::PollingMonitor defValues;
  REMUS_ASSERT( (defValues.minTimeOut() <= defValues.maxTimeOut()) );
}

void verify_changing_rates()
{
  //start with a default polling monitor
  remus::common::PollingMonitor defValues;
  remus::common::PollingMonitor ref_toDef = defValues;

  //now try to change the values to negative values, this should make
  //the min and max rate to be zero for both ref_toDef and defValues
  ref_toDef.changeTimeOutRates(-12,-1);
  REMUS_ASSERT( (ref_toDef.minTimeOut() == boost::int64_t(0)) );
  REMUS_ASSERT( (ref_toDef.maxTimeOut() == boost::int64_t(0)) );

  //verify that defValues has changed
  REMUS_ASSERT( (ref_toDef.minTimeOut() == defValues.minTimeOut()) );
  REMUS_ASSERT( (ref_toDef.maxTimeOut() == defValues.maxTimeOut()) );

  ref_toDef.changeTimeOutRates(12,1);
  REMUS_ASSERT( (ref_toDef.minTimeOut() == boost::int64_t(1)) );
  REMUS_ASSERT( (ref_toDef.maxTimeOut() == boost::int64_t(12)) );

  ref_toDef.changeTimeOutRates(-12,1);
  REMUS_ASSERT( (ref_toDef.minTimeOut() == boost::int64_t(0)) );
  REMUS_ASSERT( (ref_toDef.maxTimeOut() == boost::int64_t(1)) );

  ref_toDef.changeTimeOutRates(12,-1);
  REMUS_ASSERT( (ref_toDef.minTimeOut() == boost::int64_t(0)) );
  REMUS_ASSERT( (ref_toDef.maxTimeOut() == boost::int64_t(12)) );

  ref_toDef.changeTimeOutRates(12,1);
  REMUS_ASSERT( (ref_toDef.minTimeOut() == boost::int64_t(1)) );
  REMUS_ASSERT( (ref_toDef.maxTimeOut() == boost::int64_t(12)) );

  ref_toDef.changeTimeOutRates(100,2500000);
  REMUS_ASSERT( (ref_toDef.minTimeOut() == boost::int64_t(100)) );
  REMUS_ASSERT( (ref_toDef.maxTimeOut() == boost::int64_t(2500000)) );

  //verify that defValues has kept up with the changes.
  REMUS_ASSERT( (ref_toDef.minTimeOut() == defValues.minTimeOut()) );
  REMUS_ASSERT( (ref_toDef.maxTimeOut() == defValues.maxTimeOut()) );
}

void verify_changing_rates_and_currents_value()
{
  //Test the clamping of Current
  { //test when current is still valid in the new range
  remus::common::PollingMonitor monitor(60000,120000); //6sec - 12sec
  monitor.pollOccurred();
  boost::int64_t orig_average = monitor.average();

  REMUS_ASSERT( (monitor.current() == boost::int64_t(60000)) );
  REMUS_ASSERT( (monitor.average() <= boost::int64_t(60000)) );
  REMUS_ASSERT( (monitor.average() == orig_average) );

  //current shouldn't change since it is within the existing min,max
  monitor.changeTimeOutRates(10,120000);
  REMUS_ASSERT( (monitor.current() == boost::int64_t(60000)) );
  REMUS_ASSERT( (monitor.average() == orig_average) );
  }

  {//test that we can clamp current to a lower range
  remus::common::PollingMonitor monitor(60000,120000); //6sec - 12sec
  monitor.pollOccurred();
  boost::int64_t orig_average = monitor.average();

  //current should become 12 since that is closed to the old current
  monitor.changeTimeOutRates(10,12);
  REMUS_ASSERT( (monitor.current() == boost::int64_t(12)) );
  REMUS_ASSERT( (monitor.average() == orig_average) );
  }

  {//test that we can clamp current to a higher range
  remus::common::PollingMonitor monitor(60000,120000); //6sec - 12sec
  monitor.pollOccurred();
  boost::int64_t orig_average = monitor.average();

  //current should become 60001 since that is closed to the old current
  monitor.changeTimeOutRates(60001,120000);
  REMUS_ASSERT( (monitor.current() == boost::int64_t(60001)) );
  REMUS_ASSERT( (monitor.average() == orig_average) );
  }

}

void verify_fast_polling()
{
  boost::posix_time::ptime current =
                              boost::posix_time::microsec_clock::local_time();
  boost::posix_time::ptime t = current - boost::posix_time::hours(5);


  TestPoller p(&t);

  //verify polling at the min keeps the duration equal to the min
  const boost::int64_t minTime = p.minTimeOut();
  const boost::int64_t inputTime = (p.minTimeOut() - 1);
  for(int i=0; i < 20; ++i)
  {
  t += boost::posix_time::milliseconds(inputTime);
  p.fakeAPollOccurringAt(&t);

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
  const boost::int64_t pollTime = p.minTimeOut() +
                                 (p.maxTimeOut() - p.minTimeOut())/2;
  const long pollTimeInMSecs = static_cast<long>(pollTime);

  for(int i=0; i < 20; ++i)
  {
  t += boost::posix_time::milliseconds(pollTimeInMSecs);
  p.fakeAPollOccurringAt(&t);

  //we expect that the current will be always greater than min
  REMUS_ASSERT ( (p.current( ) > p.minTimeOut()) );
  REMUS_ASSERT ( (p.average( ) > p.minTimeOut()) );
  REMUS_ASSERT ( (p.average( ) <= pollTimeInMSecs) );
  REMUS_ASSERT ( (p.hasAbnormalEvent( ) == false) );
  }

  REMUS_ASSERT ( (p.average( ) == pollTimeInMSecs) );
  REMUS_ASSERT ( (p.hasAbnormalEvent( ) == false) );

  // verify that as we use p.current() for the duration,
  // we reach p.maxTimeOut as the current and average
  for(int i=0; i < 20; ++i)
  {
  boost::int64_t previous_current = p.current();
  boost::int64_t previous_average = p.average();
  t += boost::posix_time::milliseconds(static_cast<long>(p.current()));
  p.fakeAPollOccurringAt(&t);

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
  p.fakeAPollOccurringAt(&t);

  REMUS_ASSERT ( (p.current( ) == p.maxTimeOut()) );
  REMUS_ASSERT ( (p.average( ) > p.maxTimeOut()) );
  REMUS_ASSERT ( (p.hasAbnormalEvent( ) == true) );

  t += boost::posix_time::hours(10);
  p.fakeAPollOccurringAt(&t);

  REMUS_ASSERT ( (p.current( ) == p.maxTimeOut()) );
  REMUS_ASSERT ( (p.average( ) > p.maxTimeOut()) );
  REMUS_ASSERT ( (p.hasAbnormalEvent( ) == true) );

  while(p.current() == p.maxTimeOut())
    {
    t += boost::posix_time::milliseconds(10);
    p.fakeAPollOccurringAt(&t);
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
  verify_changing_rates();
  verify_changing_rates_and_currents_value();
  verify_fast_polling();
  verify_slow_polling();

  verify_handles_resumes();

  return 0;
}
