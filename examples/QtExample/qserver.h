/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#ifndef qserver_h
#define qserver_h


#include <QObject>
#include <QScopedPointer>
#include <QString>

namespace remus { namespace server {  class Server; }}

class qserver : public QObject
{
  Q_OBJECT
public:
  qserver();
  ~qserver();

signals:
  void started();
  void stopped();

private:
  QScopedPointer<remus::server::Server> Server;
};

#endif