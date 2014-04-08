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

#ifndef _WIN32
# include <unistd.h>
#else
#include <windows.h>
#endif

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
  //connect to the default server
  remus::worker::ServerConnection connection;
  remus::common::MeshIOType reqs =
                    remus::common::make_MeshIOType(remus::meshtypes::Model(),
                                                   remus::meshtypes::Mesh3D());
  remus::worker::Worker w(reqs,connection);
  while(this->DoWork)
    {
    std::cerr << "RWorker::startProcessing" << std::endl;
    remus::worker::Job jd = w.getJob();
    std::cerr << "got job" << std::endl;
    remus::worker::JobStatus status(jd.id(),remus::IN_PROGRESS);
    for(int progress=1; progress <= 100; ++progress)
      {
      for(int j=0; j < 200; ++j)
        {
        status.Progress.setValue(progress);
        status.Progress.setMessage(GetRandomString());
        w.updateStatus(status);
        }
      }
    remus::worker::JobResult results(jd.id(),"Job Finished");
    w.returnMeshResults(results);
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
