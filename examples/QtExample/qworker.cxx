/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "qworker.h"

#include <QTime>

#include <vector>
#include <string>
#include <iostream>

#include <remus/testing/Testing.h>
#include <remus/worker/Worker.h>

namespace
{

  std::string GetRandomString()
{
   const QString possibleCharacters("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
   const int randomStringLength = 1024*128;

   QString randomString(1024*128, 'a');
   for(int i=0; i<randomStringLength; ++i)
   {
       int index = qrand() % possibleCharacters.length();
       randomString[i] = possibleCharacters.at(index);
   }
   return randomString.toStdString();
}

}

RWorker::RWorker():
  DoWork(true),Worker()
{

}

RWorker::~RWorker()
{

}

void RWorker::startProcessing()
{
  using namespace remus::meshtypes;
  using namespace remus::proto;
  //connect to the default server
  remus::worker::ServerConnection connection;
  remus::common::MeshIOType io_type((Model()),(Mesh3D()));

  JobRequirements reqs = make_JobRequirements(io_type,"qworker","");
  remus::worker::Worker w(reqs,connection);
  while(this->DoWork)
    {
    std::cerr << "RWorker::startProcessing" << std::endl;
    remus::worker::Job jd = w.getJob();
    std::cerr << "got job" << std::endl;

    JobProgress jprogress;
    remus::proto::JobStatus status(jd.id(),remus::IN_PROGRESS);
    for(int progress=1; progress <= 100; ++progress)
      {
      for(int j=0; j < 200; ++j)
        {
        jprogress.setValue(progress);
        jprogress.setMessage(GetRandomString());
        status.updateProgress(jprogress);
        w.updateStatus(status);
        }
      }
    std::string result_data("Job Finished");
    remus::proto::JobResult results =
                            remus::proto::make_JobResult(jd.id(),result_data);
    w.returnResult(results);
    }
}

void RWorker::stopProcessing()
{
  std::cerr << "stopping the worker" << std::endl;
  this->DoWork = false;
}

qworker::qworker()
{
   RWorker *worker = new RWorker;
   worker->moveToThread(&workerThread);
   connect(&workerThread, SIGNAL(finished()), worker, SLOT(deleteLater()));
   connect(&workerThread, SIGNAL(finished()), worker, SLOT(stopProcessing()));
   connect(&workerThread, SIGNAL(started()), worker, SLOT(startProcessing()));
   workerThread.start();
}

qworker::~qworker()
{
   workerThread.quit();
   workerThread.wait();
}
