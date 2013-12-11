/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include <remus/client/Client.h>

#include <vector>
#include <iostream>

#ifndef _WIN32
  #include <limits.h>
  #include <sys/time.h>
  #include <unistd.h>
#endif

#include <boost/cstdint.hpp>

class Timer
{
public:
  Timer() { this->Reset(); }

  void Reset() { this->StartTime = this->GetCurrentTime(); }

  float GetElapsedTime()
    {
    TimeStamp currentTime = this->GetCurrentTime();
    float elapsedTime;
    elapsedTime = currentTime.Seconds - this->StartTime.Seconds;
    elapsedTime += ((currentTime.Microseconds - this->StartTime.Microseconds)
                    /float(1000000));
    return elapsedTime;
    }

  struct TimeStamp {
    boost::int64_t Seconds;
    boost::int64_t Microseconds;
  };
  TimeStamp StartTime;

  TimeStamp GetCurrentTime()
  {
    TimeStamp retval;
#ifdef _WIN32
    timeb currentTime;
    ::ftime(&currentTime);
    retval.Seconds = currentTime.time;
    retval.Microseconds = 1000*currentTime.millitm;
#else
    timeval currentTime;
    gettimeofday(&currentTime, NULL);
    retval.Seconds = currentTime.tv_sec;
    retval.Microseconds = currentTime.tv_usec;
#endif
    return retval;
  }
};

int main ()
{
  //keep track of the number of queries to do a rough test
  int queries = 0;

  //create a default server connection
  remus::client::ServerConnection conn;

  //create a client object that will connect to the default remus server
  remus::Client c(conn);

  Timer time;
  //we create a basic job request for a mesh2d job, with the data contents of "TEST"
  remus::client::JobRequest request(remus::RAW_EDGES, remus::MESH2D,
                                    "TEST THE APPLICATION BY PASSING SPACES");
  if(c.canMesh(request))
    {
    time.Reset();
    //if we can mesh 2D mesh jobs, we are going to submit 8 jobs to the server
    //for meshing
    std::vector<remus::client::Job> jobs;
    for(std::size_t i=0; i < 18; ++i, ++queries)
      {
      remus::client::Job job = c.submitJob(request);
      jobs.push_back(job);
      }

    //get the initial status of all the jobs that we have
    std::vector<remus::client::JobStatus> js;
    for(std::size_t i=0; i < jobs.size(); ++i, ++queries)
      {
      js.push_back(c.jobStatus(jobs.at(i)));
      }

    //While we have jobs still running on the server report back
    //each time the status of the job changes
    while(jobs.size() > 0)
      {
      for(std::size_t i=0; i < jobs.size(); ++i, ++queries)
        {
        //update the status with the latest status and compare it too
        //the previously held status
        remus::client::JobStatus newStatus = c.jobStatus(jobs.at(i));
        remus::client::JobStatus oldStatus = js.at(i);
        js[i]=newStatus;

        //if the status or progress value has changed report it to the cout stream
        if(newStatus.Status != oldStatus.Status ||
           newStatus.Progress != oldStatus.Progress)
          {
          std::cout << "job id " << jobs.at(i).id() << std::endl;
          if( newStatus.Status == remus::IN_PROGRESS)
            {
            std::cout << newStatus.Progress << std::endl;
            }
          else
            {
            std::cout << " status of job is: " << newStatus.Status << " " << remus::to_string(newStatus.Status)  << std::endl;
            }
          }

        //when the job has entered any of the finished states we remove it
        //from the jobs we are checking
        if( !newStatus.good() )
          {
          jobs.erase(jobs.begin()+i);
          js.erase(js.begin()+i);

          std::cout << "outstanding jobs are: " << std::endl;
          for(std::size_t j=0; j < jobs.size(); ++j)
            { std::cout << "  " << jobs.at(j).id() << std::endl; }
          }
        }
      }
    std::cout << "We issued " << queries <<  " queries to the server " << std::endl;
    std::cout << "Number of queries per second is " << queries / time.GetElapsedTime() << std::endl;
    }
  else
    {
    std::cout << "server doesn't support 2d meshes of raw triangles" << std::endl;
    }
  return 1;
}
