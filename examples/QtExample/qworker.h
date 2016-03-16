/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef qworker_h
#define qworker_h

#include <memory>
#include <utility>
#include <QObject>
#include <QThread>
#include <QScopedPointer>

namespace remus { namespace worker {  class Worker; }}


class RWorker : public QObject
{
  Q_OBJECT
  QThread workerThread;
public:
  RWorker();
  ~RWorker();

public slots:
  void stopProcessing();
  void startProcessing();
private:
  bool DoWork;
  QScopedPointer<remus::worker::Worker> Worker;
};

class qworker : public QObject
{
  Q_OBJECT
  QThread workerThread;

public:
  qworker();
  ~qworker();

signals:
  void startProcessing();
};

#endif