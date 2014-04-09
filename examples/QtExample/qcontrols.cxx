/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "qcontrols.h"
#include "ui_qcontrols.h"

#include <QtGui>
#include <QEventLoop>
#include <QThreadPool>

#include "qjob.h"

namespace
{
struct delete_pointer_element
{
  template< typename T >
  void operator()( T element ) const
  {
  if(element)
    {
    delete element;
    element=NULL;
    }
  }
};
}

//-----------------------------------------------------------------------------
qcontrols::qcontrols():
  QMainWindow(0),
  Server(),
  Workers(),
  Ui(new Ui::qcontrolsMainWindow())
{
  this->Ui->setupUi(this);
  QObject::connect(this->Ui->WorkerButton,
            SIGNAL(clicked(bool)),
            this,
            SLOT(spawnWorker()));

  QObject::connect(this->Ui->JobButton,
            SIGNAL(clicked(bool)),
            this,
            SLOT(spawnJob()));

}
qcontrols::~qcontrols()
{
  std::for_each( this->Workers.begin(),
                 this->Workers.end(),
                 delete_pointer_element() );
}

//-----------------------------------------------------------------------------
void qcontrols::closeEvent(QCloseEvent *event)
{

   QMainWindow::closeEvent(event);
   exit(0);
}

//-----------------------------------------------------------------------------
void qcontrols::spawnWorker()
{
  //spawn another worker and add the worker forever to the vector
  if(this->Workers.size() < 4)
    {
    this->Workers.push_back(new qworker());
    this->Ui->Status->setText( "Spawned a Worker" );
    }
  else
    {
    this->Ui->Status->setText( "At maximum number of workers" );
    }
}

//-----------------------------------------------------------------------------
void qcontrols::spawnJob()
{
  qjob *j = new qjob();
  QThreadPool::globalInstance()->start(j);
}
